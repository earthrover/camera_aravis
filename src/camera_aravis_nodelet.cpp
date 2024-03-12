/****************************************************************************
 *
 * camera_aravis
 *
 * Copyright © 2022 Fraunhofer IOSB and contributors
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#include <memory>
#include <unordered_set>

#define ROS_ASSERT_ENABLED
#include <ros/console.h>

#include <camera_aravis/camera_aravis_nodelet.h>

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(camera_aravis::CameraAravisNodelet, nodelet::Nodelet)

// #define ARAVIS_ERRORS_ABORT 1
#include <camera_aravis_internal/GErrorGuard.h>
#include <camera_aravis_internal/GErrorROSLog.h>
#include <camera_aravis_internal/aravis_abstraction.h>
#include <camera_aravis_internal/discover_features.h>
#include <camera_aravis_internal/resetPtpClock.h>
#include <camera_aravis_internal/tuneGVStream.h>

namespace camera_aravis {

    CameraAravisNodelet::~CameraAravisNodelet() {
        for (int i = 0; i < streams_.size(); i++) {
            if (streams_[i].arv_stream) { arv_stream_set_emit_signals(streams_[i].arv_stream.get(), FALSE); }
        }

        software_trigger_timer_.stop();

        spawning_ = false;
        if (spawn_stream_thread_.joinable()) { spawn_stream_thread_.join(); }

        for (int i = 0; i < streams_.size(); i++) {
            guint64 n_completed_buffers = 0;
            guint64 n_failures = 0;
            guint64 n_underruns = 0;
            arv_stream_get_statistics(streams_[i].arv_stream.get(), &n_completed_buffers, &n_failures, &n_underruns);
            ROS_INFO("Completed buffers = %Lu", (unsigned long long) n_completed_buffers);
            ROS_INFO("Failures          = %Lu", (unsigned long long) n_failures);
            ROS_INFO("Underruns         = %Lu", (unsigned long long) n_underruns);
            if (aravis::device::is_gv(device)) {
                guint64 n_resent;
                guint64 n_missing;
                arv_gv_stream_get_statistics(reinterpret_cast<ArvGvStream*>(streams_[i].arv_stream.get()), &n_resent,
                                             &n_missing);
                ROS_INFO("Resent buffers    = %Lu", (unsigned long long) n_resent);
                ROS_INFO("Missing           = %Lu", (unsigned long long) n_missing);
            }
        }


        if (device) {
            aravis::device::execute_command(device, "AcquisitionStop");
            aravis::device::execute_command(device, "DeviceReset");
        }
    }

#if ARAVIS_HAS_USB_MODE
    ArvUvUsbMode parse_usb_mode(const std::string& usb_mode_arg) {
        if (usb_mode_arg.size() <= 0) {
            ROS_WARN("Empty USB mode (recognized modes: SYNC, ASYNC and DEFAULT), using DEFAULT ...");
            return ARV_UV_USB_MODE_DEFAULT;
        }
        if (usb_mode_arg[0] == 's' or usb_mode_arg[0] == 'S') {
            return ARV_UV_USB_MODE_SYNC;
        } else if (usb_mode_arg[0] == 'a' or usb_mode_arg[0] == 'A') {
            return ARV_UV_USB_MODE_ASYNC;
        } else if (usb_mode_arg[0] == 'd' or usb_mode_arg[0] == 'D') {
            return ARV_UV_USB_MODE_DEFAULT;
        }

        ROS_WARN_STREAM("Unrecognized USB mode " << usb_mode_arg
                                                 << " (recognized modes: SYNC, ASYNC and DEFAULT), using DEFAULT ...");
        return ARV_UV_USB_MODE_DEFAULT;
    }
#endif

    std::string get_tf_prefix(const ros::NodeHandle& nh) {
        std::string tf_prefix = "";
        std::string tf_prefix_location;
        if (nh.searchParam("tf_prefix", tf_prefix_location)) {
            nh.getParam(tf_prefix_location, tf_prefix);
            if (!tf_prefix.empty()) tf_prefix += "/";
        }
        return tf_prefix;
    }

    void CameraAravisNodelet::onInit() {
        ros::NodeHandle pnh = getPrivateNodeHandle();

        // Retrieve ros parameters
        // Get the camera guid as a parameter or use the first device.
        guid_ = pnh.param<std::string>("guid", guid_);
        use_ptp_stamp_ = pnh.param<bool>("use_ptp_timestamp", use_ptp_stamp_);

        double software_trigger_rate = pnh.param<double>("software_trigger_rate", 0);
        frame_id_ = get_tf_prefix(pnh) + pnh.param<std::string>("frame_id", frame_id_);

        std::string stream_channel_args;
        if (pnh.getParam("channel_names", stream_channel_args)) {
            parseStringArgs(stream_channel_args, stream_names_);
        } else {
            stream_names_ = {""};
        }

        std::string pixel_format_args;
        std::vector<std::string> pixel_formats;
        pnh.param("PixelFormat", pixel_format_args, pixel_format_args);
        parseStringArgs(pixel_format_args, pixel_formats);

        std::string calib_url_args;
        std::vector<std::string> calib_urls;
        pnh.param("camera_info_url", calib_url_args, calib_url_args);
        parseStringArgs(calib_url_args, calib_urls);

        // Print out some useful info.
        ROS_INFO("Attached cameras:");
        arv_update_device_list();
        uint n_interfaces = arv_get_n_interfaces();
        ROS_INFO("# Interfaces: %d", n_interfaces);

        uint n_devices = arv_get_n_devices();
        for (int i = 5; i > 0; --i) {
            if (n_devices == 0) {
                ROS_ERROR("No cameras detected, retrying in 1s ...");
                ros::Duration(1.0).sleep();
                n_devices = arv_get_n_devices();
            }
        }

        if (n_devices == 0) {
            ROS_FATAL("No cameras detected, exiting.");
            shutdown();
        }

        ROS_INFO("# Devices: %d", n_devices);
        for (uint i = 0; i < n_devices; i++) ROS_INFO("Device%d: %s", i, arv_get_device_id(i));

        // Open the camera, and set it up.
        while (!camera) {
            if (guid_.empty()) {
                ROS_INFO("Opening: (any)");
                camera = aravis::camera_new();
            } else {
                ROS_INFO_STREAM("Opening: " << guid_);
                camera = aravis::camera_new(guid_.c_str());
            }
            ros::Duration(1.0).sleep();
        }

        device = aravis::camera::get_device(camera);
        ROS_INFO("Opened: %s-%s", aravis::camera::get_vendor_name(camera),
                 aravis::device::feature::get_string(device, "DeviceSerialNumber"));

        // See which features exist in this camera device
        implemented_features_ = internal::discover_features(device);

        // Check the number of streams for this camera
        num_streams_ = aravis::device::get_num_streams(device);
        // if this also returns 0, assume number of streams = 1
        if (!num_streams_) {
            ROS_WARN("Unable to detect number of supported stream channels, assuming 1 ...");
            num_streams_ = 1;
        }

        ROS_INFO("Number of supported stream channels %i.", (int) num_streams_);

        // check if every stream channel has been given a channel name
        if (stream_names_.size() < num_streams_) { num_streams_ = stream_names_.size(); }


#if ARAVIS_HAS_USB_MODE
        ArvUvUsbMode usb_mode = parse_usb_mode(pnh.param<std::string>("usb_mode", "default"));
        aravis::device::USB3Vision::set_usb_mode(device, usb_mode);
#endif

        if (pnh.param("load_user_set", false)) {
            std::string user_set;
            if (pnh.getParam("UserSetSelector", user_set)) {
                aravis::device::feature::set_string(device, "UserSetSelector", user_set.c_str());
            }
            aravis::device::execute_command(device, "UserSetLoad");
        }

        aravis::camera::bounds::get_width(camera, &roi_.width_min, &roi_.width_max);
        aravis::camera::bounds::get_height(camera, &roi_.height_min, &roi_.height_max);


        streams_.resize(num_streams_);
        stream_ids_.resize(num_streams_);

        for (int i = 0; i < num_streams_; i++) {
            stream_ids_[i].can = this;
            stream_ids_[i].stream_id = i;

            Stream& stream = streams_[i];
            if (stream_names_.size() > i) { stream.name = stream_names_.at(i); }

            if (aravis::device::is_gv(device)) { aravis::camera::gv::select_stream_channel(camera, i); }

            aravis::camera::get_sensor_size(camera, &stream.sensor_description.width,
                                            &stream.sensor_description.height);

            // Initial camera settings.

            // init default to full sensor resolution
            aravis::camera::set_region(camera, 0, 0, roi_.width_max, roi_.height_max);

            // Set up the triggering.
            if (implemented_features_["TriggerMode"] && implemented_features_["TriggerSelector"]) {
                aravis::device::feature::set_string(device, "TriggerSelector", "FrameStart");
                aravis::device::feature::set_string(device, "TriggerMode", "Off");
            }

            // possibly set or override from given parameter
            writeCameraFeaturesFromRosparam();

            std::string source_selector = "Source" + std::to_string(i);

            if (implemented_features_["SourceSelector"]) {
                aravis::device::feature::set_string(device, "SourceSelector", source_selector.c_str());
            }

            if (implemented_features_["PixelFormat"] && pixel_formats[i].size()) {
                aravis::device::feature::set_string(device, "PixelFormat", pixel_formats[i].c_str());
            }

            if (implemented_features_["PixelFormat"]) {
                stream.sensor_description.pixel_format =
                    std::string(aravis::device::feature::get_string(device, "PixelFormat"));

                stream.sensor_description.n_bits_pixel =
                    ARV_PIXEL_FORMAT_BIT_PER_PIXEL(aravis::device::feature::get_integer(device, "PixelFormat"));
            }

            const auto& iter = CONVERSIONS_DICTIONARY.find(stream.sensor_description.pixel_format);
            if (iter != CONVERSIONS_DICTIONARY.end()) {
                // convert_formats.push_back(iter->second);
                stream.conversion_function = iter->second;
            } else {
                ROS_WARN_STREAM("There is no known conversion from "
                                << stream.sensor_description.pixel_format
                                << " to a usual ROS image encoding. Likely you need to implement one.");
            }

            // Start the camerainfo manager.
            std::string camera_info_frame_id = frame_id_;
            if (!stream_names_[i].empty()) camera_info_frame_id = frame_id_ + '/' + stream_names_[i];

            stream.camera_info_node_handle = pnh;
            // Use separate node handles for CameraInfoManagers when using a Multisource Camera
            if (!stream_names_[i].empty()) { stream.camera_info_node_handle = ros::NodeHandle(pnh, stream_names_[i]); }

            stream.camera_info_manager = std::make_unique<camera_info_manager::CameraInfoManager>(
                stream.camera_info_node_handle, camera_info_frame_id, calib_urls[i]);


            ROS_INFO("Reset %s Camera Info Manager", stream_names_[i].c_str());
            ROS_INFO("%s Calib URL: %s", stream_names_[i].c_str(), calib_urls[i].c_str());
        }

        // get current state of camera for config_
        aravis::camera::get_region(camera, &roi_.x, &roi_.y, &roi_.width, &roi_.height);

        // Print information.
        print_capabilities();


        // Reset PTP clock
        if (use_ptp_stamp_) { internal::resetPtpClock(device); }

        // spawn camera stream in thread, so onInit() is not blocked
        spawning_ = true;
        spawn_stream_thread_ = std::thread(&CameraAravisNodelet::spawnStream, this);

        // activate on demand
        if (software_trigger_rate > 0) {
            if (implemented_features_["TriggerSoftware"]) {
                ROS_INFO("Set softwaretriggerrate = %f", 1000.0 / ceil(1000.0 / software_trigger_rate));

                software_trigger_timer_ =
                    pnh.createTimer(ros::Duration(ros::Rate(software_trigger_rate)), [&](const ros::TimerEvent& evt) {
                        if (std::any_of(streams_.cbegin(), streams_.cend(), [](const Stream& stream) {
                                return stream.camera_publisher.getNumSubscribers() > 0;
                            })) {
                            aravis::device::execute_command(device, "TriggerSoftware");
                            ROS_ERROR("Software trigger");
                        }

                        const auto period = evt.current_expected - evt.last_expected;
                        const auto time_diff = evt.current_real - evt.current_expected;

                        if (time_diff.toSec() > period.toSec() * 0.1) {
                            ROS_WARN("Camera Aravis: Missed a software trigger event by %fs.", time_diff.toSec());
                        }
                    });
            } else {
                ROS_INFO("Camera does not support TriggerSoftware command.");
            }
        }
    }

    void CameraAravisNodelet::spawnStream() {
        ros::NodeHandle nh = getNodeHandle();
        ros::NodeHandle pnh = getPrivateNodeHandle();
        GuardedGError error;

        // Monitor whether anyone is subscribed to the camera stream
        image_transport::SubscriberStatusCallback image_cb =
            [this](const image_transport::SingleSubscriberPublisher& ssp) { this->rosConnectCallback(); };

        ros::SubscriberStatusCallback info_cb = [this](const ros::SingleSubscriberPublisher& ssp) {
            this->rosConnectCallback();
        };

        image_transport::ImageTransport p_transport(pnh);

        for (int i = 0; i < num_streams_; i++) {
            Stream& stream = streams_[i];
            while (spawning_) {
                if (aravis::device::is_gv(device)) { aravis::camera::gv::select_stream_channel(camera, i); }

                stream.arv_stream = aravis::camera::create_stream(camera, NULL, NULL);  // TODO: Use smart pointer

                if (stream.arv_stream) { break; }

                ROS_WARN("Stream %i: Could not create image stream for %s.  Retrying...", i, guid_.c_str());
                ros::Duration(1.0).sleep();
                ros::spinOnce();
            }

            // Load up some buffers.
            if (aravis::device::is_gv(device)) aravis::camera::gv::select_stream_channel(camera, i);

            const gint64 n_bytes_payload_stream = aravis::camera::get_payload(camera);

            stream.buffer_pool = boost::make_shared<CameraBufferPool>(stream.arv_stream.get(), n_bytes_payload_stream, 10);

            if (aravis::device::is_gv(device)) {
                internal::tuneGvStream(reinterpret_cast<ArvGvStream*>(stream.arv_stream.get()));
            }

            // Set up image_raw
            std::string topic_name = this->getName();
            if (num_streams_ != 1 || !stream_names_[i].empty()) { topic_name += "/" + stream_names_[i]; }

            stream.camera_publisher = p_transport.advertiseCamera(ros::names::remap(topic_name + "/image_raw"), 1,
                                                                  image_cb, image_cb, info_cb, info_cb);

            // Connect signals with callbacks.
            g_signal_connect(
                stream.arv_stream.get(), "new-buffer",
                (GCallback) +
                    [](ArvStream* p_stream, gpointer id_instance) {
                        // workaround to get access to the instance from a static method
                        StreamIdData* data = (StreamIdData*) id_instance;

                        Stream& stream = data->can->streams_[data->stream_id];

                        const std::string stream_frame_id =
                            stream.name.empty() ? data->can->frame_id_ : data->can->frame_id_ + "/" + stream.name;

                        newBufferReady(stream, stream_frame_id, data->can->roi_.width, data->can->roi_.height,
                                       data->can->use_ptp_stamp_);

                        // check PTP status, camera cannot recover from "Faulty" by itself
                        if (data->can->use_ptp_stamp_) internal::resetPtpClock(data->can->device);
                    },
                &(stream_ids_[i]));
        }
        g_signal_connect(device.get(), "control-lost", (GCallback) CameraAravisNodelet::controlLostCallback, this);

        for (int i = 0; i < num_streams_; i++) { arv_stream_set_emit_signals(streams_[i].arv_stream.get(), TRUE); }

        if (std::any_of(streams_.cbegin(), streams_.cend(),
                        [](const Stream& stream) { return stream.camera_publisher.getNumSubscribers() > 0; })) {
            aravis::camera::start_acquisition(camera);
        }

        this->get_integer_service_ =
            pnh.advertiseService("get_integer_feature_value", &CameraAravisNodelet::getIntegerFeatureCallback, this);
        this->get_float_service_ =
            pnh.advertiseService("get_float_feature_value", &CameraAravisNodelet::getFloatFeatureCallback, this);
        this->get_string_service_ =
            pnh.advertiseService("get_string_feature_value", &CameraAravisNodelet::getStringFeatureCallback, this);
        this->get_boolean_service_ =
            pnh.advertiseService("get_boolean_feature_value", &CameraAravisNodelet::getBooleanFeatureCallback, this);

        this->set_integer_service_ =
            pnh.advertiseService("set_integer_feature_value", &CameraAravisNodelet::setIntegerFeatureCallback, this);
        this->set_float_service_ =
            pnh.advertiseService("set_float_feature_value", &CameraAravisNodelet::setFloatFeatureCallback, this);
        this->set_string_service_ =
            pnh.advertiseService("set_string_feature_value", &CameraAravisNodelet::setStringFeatureCallback, this);
        this->set_boolean_service_ =
            pnh.advertiseService("set_boolean_feature_value", &CameraAravisNodelet::setBooleanFeatureCallback, this);

        this->exec_command_service_ =
            pnh.advertiseService("execute_command", &CameraAravisNodelet::executeCommandCallback, this);

        ROS_INFO("Done initializing camera_aravis.");
    }

    void CameraAravisNodelet::rosConnectCallback() {
        if (static_cast<bool>(device)) {
            if (std::all_of(streams_.cbegin(), streams_.cend(),
                            [](const Stream& stream) { return stream.camera_publisher.getNumSubscribers() == 0; })) {
                // don't waste CPU if nobody is listening!
                aravis::device::execute_command(device, "AcquisitionStop");
            } else {
                aravis::device::execute_command(device, "AcquisitionStart");
            }
        }
    }

    void CameraAravisNodelet::newBufferReady(Stream& stream,
                                             std::string frame_id,
                                             int32_t width,
                                             int32_t height,
                                             bool use_ptp_stamp) {
        ArvBuffer* p_buffer = arv_stream_try_pop_buffer(stream.arv_stream.get());

        // check if we risk to drop the next image because of not enough buffers left
        gint n_available_buffers;
        arv_stream_get_n_buffers(stream.arv_stream.get(), &n_available_buffers, NULL);


        if (n_available_buffers == 0) { stream.buffer_pool->allocateBuffers(1); }

        if (p_buffer == NULL) { return; }

        if (arv_buffer_get_status(p_buffer) != ARV_BUFFER_STATUS_SUCCESS || !stream.buffer_pool ||
            stream.camera_publisher.getNumSubscribers() == 0) {
            if (arv_buffer_get_status(p_buffer) != ARV_BUFFER_STATUS_SUCCESS) {
                ROS_WARN("(%s) Buffer error: %s", frame_id.c_str(),
                         aravis::buffer::status_string(arv_buffer_get_status(p_buffer)));
            }

            arv_stream_push_buffer(stream.arv_stream.get(), p_buffer);
            return;
        }

        // get the image message which wraps around this buffer
        sensor_msgs::ImagePtr msg_ptr = (*(stream.buffer_pool))[p_buffer];

        // fill the meta information of image message
        // get acquisition time
        guint64 t = (use_ptp_stamp) ? arv_buffer_get_timestamp(p_buffer) : arv_buffer_get_system_timestamp(p_buffer);

        msg_ptr->header.stamp.fromNSec(t);
        // get frame sequence number
        msg_ptr->header.seq = arv_buffer_get_frame_id(p_buffer);
        // fill other stream properties
        msg_ptr->header.frame_id = frame_id;
        msg_ptr->width = width;
        msg_ptr->height = height;
        msg_ptr->encoding = stream.sensor_description.pixel_format;
        msg_ptr->step = (msg_ptr->width * stream.sensor_description.n_bits_pixel) / 8;

        // do the magic of conversion into a ROS format
        if (stream.conversion_function) {
            sensor_msgs::ImagePtr cvt_msg_ptr = stream.buffer_pool->getRecyclableImg();
            stream.conversion_function(msg_ptr, cvt_msg_ptr);
            msg_ptr = cvt_msg_ptr;
        }

        // get current CameraInfo data
        stream.camera_info = boost::make_shared<sensor_msgs::CameraInfo>(stream.camera_info_manager->getCameraInfo());

        stream.camera_info->header = msg_ptr->header;

        if (stream.camera_info->width == 0 || stream.camera_info->height == 0) {
            ROS_WARN_STREAM_ONCE("The fields image_width and image_height seem not to be set in "
                                 "the YAML specified by 'camera_info_url' parameter. Please set "
                                 "them there, because actual image size and specified image size "
                                 "can be different due to the region of interest (ROI) feature. In "
                                 "the YAML the image size should be the one on which the camera was "
                                 "calibrated. See CameraInfo.msg specification!");

            stream.camera_info->width = width;
            stream.camera_info->height = height;
        }

        stream.camera_publisher.publish(msg_ptr, stream.camera_info);
    }

    void CameraAravisNodelet::controlLostCallback(ArvDevice* p_gv_device, gpointer can_instance) {
        auto* p_can = static_cast<CameraAravisNodelet*>(can_instance);
        ROS_ERROR("Control to aravis device (%s) lost.", p_can->guid_.c_str());
        p_can->shutdown();
    }

    void CameraAravisNodelet::shutdown() {
        nodelet::NodeletUnload unload_service;
        unload_service.request.name = this->getName();
        if (!ros::service::call(ros::this_node::getName() + "/unload_nodelet", unload_service)) { ros::shutdown(); }
    }

    void CameraAravisNodelet::parseStringArgs(const std::string& in_arg_string, std::vector<std::string>& out_args) {
        size_t array_start = 0;
        size_t array_end = in_arg_string.length();
        if (array_start != std::string::npos && array_end != std::string::npos) {
            // parse the string into an array of parameters
            std::stringstream ss(in_arg_string.substr(array_start, array_end - array_start));
            while (ss.good()) {
                std::string temp;
                getline(ss, temp, ',');
                boost::trim_left(temp);
                boost::trim_right(temp);
                out_args.push_back(temp);
            }
        } else {
            // add just the one argument onto the vector
            out_args.push_back(in_arg_string);
        }
    }

    // WriteCameraFeaturesFromRosparam()
    // Read ROS parameters from this node's namespace, and see if each parameter has a similarly named & typed feature
    // in the camera.  Then set the camera feature to that value.  For example, if the parameter camnode/Gain is set to
    // 123.0, then we'll write 123.0 to the Gain feature in the camera.
    //
    // Note that the datatype of the parameter *must* match the datatype of the camera feature, and this can be
    // determined by looking at the camera's XML file.  Camera enum's are string parameters, camera bools are false/true
    // parameters (not 0/1), integers are integers, doubles are doubles, etc.
    //
    void CameraAravisNodelet::writeCameraFeaturesFromRosparam() {
        XmlRpc::XmlRpcValue xml_rpc_params;
        XmlRpc::XmlRpcValue::iterator iter;

        getPrivateNodeHandle().getParam(this->getName() + "/feature_load_order", xml_rpc_params);
        if (xml_rpc_params.getType() == XmlRpc::XmlRpcValue::TypeArray) {
            for (int32_t i = 0; i < xml_rpc_params.size(); ++i) {
                const auto& elem = xml_rpc_params[i];
                if (elem.getType() != XmlRpc::XmlRpcValue::TypeString) {
                    ROS_WARN_STREAM("Invalid value '" << std::string(elem) << "' in param: " << this->getName()
                                                      << "/feature_load_order");
                    return;
                }

                XmlRpc::XmlRpcValue item;
                getPrivateNodeHandle().getParam(this->getName() + "/" + static_cast<std::string>(elem), item);
                this->writeCameraFeatureFromRosparam(std::make_pair(static_cast<std::string>(elem), item));
            }
        }

        getPrivateNodeHandle().getParam(this->getName(), xml_rpc_params);

        if (xml_rpc_params.getType() != XmlRpc::XmlRpcValue::TypeStruct) { return; }

        // Write first the Selectors given that no meaningful order could be applied (cascade order is lost)
        std::for_each(xml_rpc_params.begin(), xml_rpc_params.end(), [&](auto elem) {
            if (elem.first.find("Selector") != elem.first.npos) this->writeCameraFeatureFromRosparam(elem);
        });

        std::for_each(xml_rpc_params.begin(), xml_rpc_params.end(), [&](auto elem) {
            if (elem.first.find("Selector") == elem.first.npos) this->writeCameraFeatureFromRosparam(elem);
        });
    }

    void CameraAravisNodelet::writeCameraFeatureFromRosparam(const XmlRpc::XmlRpcValue::iterator::value_type& iter) {
        GError* error = NULL;

        std::string key = iter.first;

        ArvGcNode* p_gc_node = arv_device_get_feature(device.get(), key.c_str());
        if (p_gc_node && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(p_gc_node), &error)) {

            // We'd like to check the value types too, but typeValue is often given as G_TYPE_INVALID, so ignore it.
            switch (iter.second.getType()) {
                case XmlRpc::XmlRpcValue::TypeBoolean:
                    {
                        bool value = (bool) iter.second;
                        aravis::device::feature::set_boolean(device, key.c_str(), value);
                        ROS_INFO("Read parameter (bool) %s: %s", key.c_str(), value ? "true" : "false");
                    }
                    break;

                case XmlRpc::XmlRpcValue::TypeInt:
                    {
                        int value = (int) iter.second;
                        aravis::device::feature::set_integer(device, key.c_str(), value);
                        ROS_INFO("Read parameter (int) %s: %d", key.c_str(), value);
                    }
                    break;

                case XmlRpc::XmlRpcValue::TypeDouble:
                    {
                        double value = (double) iter.second;
                        aravis::device::feature::set_float(device, key.c_str(), value);
                        ROS_INFO("Read parameter (float) %s: %f", key.c_str(), value);
                    }
                    break;

                case XmlRpc::XmlRpcValue::TypeString:
                    {
                        std::string value = (std::string) iter.second;
                        aravis::device::feature::set_string(device, key.c_str(), value.c_str());
                        ROS_INFO("Read parameter (string) %s: %s", key.c_str(), value.c_str());
                    }
                    break;

                case XmlRpc::XmlRpcValue::TypeInvalid:
                case XmlRpc::XmlRpcValue::TypeDateTime:
                case XmlRpc::XmlRpcValue::TypeBase64:
                case XmlRpc::XmlRpcValue::TypeArray:
                case XmlRpc::XmlRpcValue::TypeStruct:
                default:
                    ROS_WARN("Unhandled rosparam type in writeCameraFeaturesFromRosparam(), feature: %s", key.c_str());
            }
        }
    }

}  // end namespace camera_aravis
