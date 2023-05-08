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

#ifndef ARAVIS_HAS_USB_MODE
#if ARAVIS_MAJOR_VERSION > 0 || \
    ARAVIS_MAJOR_VERSION == 0 && ARAVIS_MINOR_VERSION > 8 || \
    ARAVIS_MAJOR_VERSION == 0 && ARAVIS_MINOR_VERSION == 8 && ARAVIS_MICRO_VERSION >= 17
  #define ARAVIS_HAS_USB_MODE 1
#else
  #define ARAVIS_HAS_USB_MODE 0
#endif
#endif


// #define ARAVIS_ERRORS_ABORT 1

#ifdef ARAVIS_ERRORS_ABORT
#define LOG_GERROR_ARAVIS(err)                                                                         \
    ROS_ASSERT_MSG((err) == nullptr, "%s: [%s] Code %i: %s", ::camera_aravis::aravis::logger_suffix.c_str(), \
               g_quark_to_string((err)->domain), (err)->code, (err)->message)
#else
#define LOG_GERROR_ARAVIS(err)                                                                           \
    ROS_ERROR_COND_NAMED((err) != nullptr, ::camera_aravis::aravis::logger_suffix, "[%s] Code %i: %s", \
                             g_quark_to_string((err)->domain), (err)->code, (err)->message)
#endif

namespace camera_aravis
{
  using GErrorGuard = std::unique_ptr<GError*, void (*)(GError**)>;

  GErrorGuard makeGErrorGuard() {
      return GErrorGuard(nullptr, [](GErrorGuard::pointer error) {
          if (error && *error) g_error_free(*error);
      });
  }

  class GuardedGError {
    public:
    ~GuardedGError() { reset(); }

    void reset() {
      if (!err) return;
      g_error_free(err);
      err = nullptr;
    }

    GError** storeError() { return &err; }

    GError* operator->() noexcept { return err; }

    operator bool() const { return nullptr != err; }

    void log(const std::string& suffix = "") {
        bool cond = *this == nullptr;
        ROS_ERROR_COND_NAMED(err != nullptr, suffix, "[%s] Code %i: %s", g_quark_to_string(err->domain), err->code, err->message);
    }

    friend bool operator==(const GuardedGError& lhs, const GError* rhs);
    friend bool operator==(const GuardedGError& lhs, const GuardedGError& rhs);
    friend bool operator!=(const GuardedGError& lhs, std::nullptr_t);

    private:

    GError *err = nullptr;
  };

  bool operator==(const GuardedGError& lhs, const GError* rhs) { return lhs.err == rhs; }
  bool operator==(const GuardedGError& lhs, const GuardedGError& rhs) { return lhs.err == rhs.err; }
  bool operator!=(const GuardedGError& lhs, std::nullptr_t) { return !!lhs; }

namespace aravis {
  const std::string logger_suffix = "aravis";

  namespace device {
    void execute_command(ArvDevice* dev, const char* cmd) {
        GuardedGError err;
        arv_device_execute_command(dev, cmd, err.storeError());
        LOG_GERROR_ARAVIS(err);
    }

    namespace feature {          
      gboolean get_boolean(ArvDevice* dev, const char* feat) {
        GuardedGError err;
        gboolean res = arv_device_get_boolean_feature_value(dev, feat, err.storeError());
        LOG_GERROR_ARAVIS(err);
        return res;
      }

      void set_boolean(ArvDevice* dev, const char* feat, gboolean val) {
        GuardedGError err;
        arv_device_set_boolean_feature_value(dev, feat, val, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      gint64 get_integer(ArvDevice* dev, const char* feat) {
        GuardedGError err;
        gint64 res = arv_device_get_integer_feature_value(dev, feat, err.storeError());
        LOG_GERROR_ARAVIS(err);
        return res;
      }

      void set_integer(ArvDevice* dev, const char* feat, gint64 val) {
        GuardedGError err;
        arv_device_set_integer_feature_value(dev, feat, val, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      double get_float(ArvDevice* dev, const char* feat) {
        GuardedGError err;
        double res = arv_device_get_float_feature_value(dev, feat, err.storeError());
        LOG_GERROR_ARAVIS(err);
        return res;
      }

      void set_float(ArvDevice* dev, const char* feat, double val) {
        GuardedGError err;
        arv_device_set_float_feature_value(dev, feat, val, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      const char* get_string(ArvDevice* dev, const char* feat) {
        GuardedGError err;
        const char* res = arv_device_get_string_feature_value(dev, feat, err.storeError());
        LOG_GERROR_ARAVIS(err);
        return res;
      }

      void set_string(ArvDevice* dev, const char* feat, const char* val) {
        GuardedGError err;
        arv_device_set_string_feature_value(dev, feat, val, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      namespace bounds {
        void get_integer(ArvDevice* dev, const char* feat, gint64* min, gint64* max) {
          GuardedGError err;
          arv_device_get_integer_feature_bounds(dev, feat, min, max, err.storeError());
          LOG_GERROR_ARAVIS(err);
        }

        void get_float(ArvDevice* dev, const char* feat, double* min, double* max) {
            GuardedGError err;
            arv_device_get_float_feature_bounds(dev, feat, min, max, err.storeError());
            LOG_GERROR_ARAVIS(err);
        }
      }
    }
  }

  ArvCamera* camera_new (const char* name = NULL) {
    GuardedGError err;
    ArvCamera* res = arv_camera_new (name, err.storeError());
    // LOG_GERROR_ARAVIS(err);
    err.log(logger_suffix);
    return res;
  }

  namespace camera {

    const char* get_vendor_name(ArvCamera *cam) {
      GuardedGError err;
      const char* res = arv_camera_get_vendor_name(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    gint64 get_payload(ArvCamera *cam) {
      GuardedGError err;
      gint64 res = arv_camera_get_payload(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    double get_frame_rate(ArvCamera *cam) {
      GuardedGError err;
      double res = arv_camera_get_frame_rate(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    void set_frame_rate(ArvCamera *cam, double val) {
      GuardedGError err;
      arv_camera_set_frame_rate(cam, val, err.storeError());
      LOG_GERROR_ARAVIS(err);
    }

    double get_exposure_time(ArvCamera *cam){
      GuardedGError err;
      double res = arv_camera_get_exposure_time(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    void set_exposure_time(ArvCamera *cam, double val){
      GuardedGError err;
      arv_camera_set_exposure_time(cam, val, err.storeError());
      LOG_GERROR_ARAVIS(err);
    }

    double get_gain(ArvCamera *cam){
      GuardedGError err;
      double res = arv_camera_get_gain(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    void set_gain(ArvCamera *cam, double val){
      GuardedGError err;
      arv_camera_set_gain(cam, val, err.storeError());
      LOG_GERROR_ARAVIS(err);
    }

    void get_region(ArvCamera* cam, gint* x, gint* y, gint* width, gint* height) {
        GuardedGError err;
        arv_camera_get_region(cam, x, y, width, height, err.storeError());
        LOG_GERROR_ARAVIS(err);
    }

    void set_region(ArvCamera* cam, gint x, gint y, gint width, gint height) {
      GuardedGError err;
      arv_camera_set_region(cam, x, y, width, height, err.storeError());
      LOG_GERROR_ARAVIS(err);
    }

    void get_sensor_size(ArvCamera* cam, gint* width, gint* height) {
        GuardedGError err;
        arv_camera_get_sensor_size(cam, width, height, err.storeError());
        LOG_GERROR_ARAVIS(err);
    }

    ArvStream* create_stream(ArvCamera* cam, ArvStreamCallback callback, void* user_data) {
      GuardedGError err;
      ArvStream* res = arv_camera_create_stream(cam, callback, user_data, err.storeError());
      LOG_GERROR_ARAVIS(err);
      return res;
    }

    void start_acquisition(ArvCamera* cam) {
      GuardedGError err;
      arv_camera_start_acquisition(cam, err.storeError());
      LOG_GERROR_ARAVIS(err);
    }


    namespace bounds {

      void get_width(ArvCamera *cam, gint* min, gint* max) {
        GuardedGError err;
        arv_camera_get_width_bounds(cam, min, max, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      void get_height(ArvCamera *cam, gint* min, gint* max) {
        GuardedGError err;
        arv_camera_get_height_bounds(cam, min, max, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      void get_exposure_time(ArvCamera *cam, double* min, double* max) {
        GuardedGError err;
        arv_camera_get_exposure_time_bounds(cam, min, max, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      void get_gain(ArvCamera *cam, double* min, double* max) {
        GuardedGError err;
        arv_camera_get_gain_bounds(cam, min, max, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }

      void get_frame_rate(ArvCamera *cam, double* min, double* max) {
        GuardedGError err;
        arv_camera_get_frame_rate_bounds(cam, min, max, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }


    }

    namespace gv {
      void select_stream_channel(ArvCamera* cam, gint channel_id){
        GuardedGError err;
        arv_camera_gv_select_stream_channel(cam, channel_id, err.storeError());
        LOG_GERROR_ARAVIS(err);
      }
    }
  }
}

CameraAravisNodelet::CameraAravisNodelet()
{
}

CameraAravisNodelet::~CameraAravisNodelet()
{
  for(int i=0; i < p_streams_.size(); i++) {
    if(p_streams_[i]) {
      arv_stream_set_emit_signals(p_streams_[i], FALSE);
    }
  }

  spawning_ = false;
  if (spawn_stream_thread_.joinable())
  {
    spawn_stream_thread_.join();
  }

  software_trigger_active_ = false;
  if (software_trigger_thread_.joinable())
  {
    software_trigger_thread_.join();
  }

  tf_thread_active_ = false;
  if (tf_dyn_thread_.joinable())
  {
    tf_dyn_thread_.join();
  }


  for(int i=0; i < p_streams_.size(); i++) {
    guint64 n_completed_buffers = 0;
    guint64 n_failures = 0;
    guint64 n_underruns = 0;
    arv_stream_get_statistics(p_streams_[i], &n_completed_buffers, &n_failures, &n_underruns);
    ROS_INFO("Completed buffers = %Lu", (unsigned long long ) n_completed_buffers);
    ROS_INFO("Failures          = %Lu", (unsigned long long ) n_failures);
    ROS_INFO("Underruns         = %Lu", (unsigned long long ) n_underruns);
    if (arv_camera_is_gv_device(p_camera_))
    {
    guint64 n_resent;
    guint64 n_missing;
    arv_gv_stream_get_statistics(reinterpret_cast<ArvGvStream*>(p_streams_[i]), &n_resent, &n_missing);
    ROS_INFO("Resent buffers    = %Lu", (unsigned long long ) n_resent);
    ROS_INFO("Missing           = %Lu", (unsigned long long ) n_missing);
    }
  }


  if (p_device_)
  {
    aravis::device::execute_command(p_device_, "AcquisitionStop");
  }

  for(int i = 0; i < p_streams_.size(); i++) {
      g_object_unref(p_streams_[i]);
  }
  g_object_unref(p_camera_);
}

void CameraAravisNodelet::onInit()
{
  ros::NodeHandle pnh = getPrivateNodeHandle();

  // Retrieve ros parameters
  verbose_ = pnh.param<bool>("verbose", verbose_);
  guid_ = pnh.param<std::string>("guid", guid_); // Get the camera guid as a parameter or use the first device.
  use_ptp_stamp_ = pnh.param<bool>("use_ptp_timestamp", use_ptp_stamp_);
  pub_ext_camera_info_ = pnh.param<bool>("ExtendedCameraInfo", pub_ext_camera_info_); // publish an extended camera info message
  pub_tf_optical_ = pnh.param<bool>("publish_tf", pub_tf_optical_); // should we publish tf transforms to camera optical frame?

  std::string stream_channel_args;
  if (pnh.getParam("channel_names", stream_channel_args)) {
    parseStringArgs(stream_channel_args, stream_names_);
  } else {
    stream_names_ = { "" };
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
  ROS_INFO("# Devices: %d", n_devices);
  for (uint i = 0; i < n_devices; i++)
    ROS_INFO("Device%d: %s", i, arv_get_device_id(i));

  if (n_devices == 0)
  {
    ROS_ERROR("No cameras detected.");
    return;
  }

  // Open the camera, and set it up.
  while (!p_camera_)
  {
    if (guid_.empty())
    {
      ROS_INFO("Opening: (any)");
      p_camera_ = aravis::camera_new();
    }
    else
    {
      ROS_INFO_STREAM("Opening: " << guid_);
      p_camera_ = aravis::camera_new(guid_.c_str());
    }
    ros::Duration(1.0).sleep();
  }

  p_device_ = arv_camera_get_device(p_camera_);
  ROS_INFO("Opened: %s-%s", aravis::camera::get_vendor_name(p_camera_),
           aravis::device::feature::get_string(p_device_, "DeviceSerialNumber"));

  // Start the dynamic_reconfigure server.
  reconfigure_server_.reset(new dynamic_reconfigure::Server<Config>(reconfigure_mutex_, pnh));
  reconfigure_server_->getConfigDefault(config_);
  reconfigure_server_->getConfigMin(config_min_);
  reconfigure_server_->getConfigMax(config_max_);

  // See which features exist in this camera device
  discoverFeatures();

  // Check the number of streams for this camera
  num_streams_ = arv_device_get_integer_feature_value(p_device_, "DeviceStreamChannelCount", nullptr);
  // if this return 0, try the deprecated GevStreamChannelCount in case this is an older camera
  if (!num_streams_ && arv_camera_is_gv_device(p_camera_)) {
    num_streams_ = arv_device_get_integer_feature_value(p_device_, "GevStreamChannelCount", nullptr);
  }
  // if this also returns 0, assume number of streams = 1
  if (!num_streams_) {
    ROS_WARN("Unable to detect number of supported stream channels, assuming 1 ...");
    num_streams_ = 1;
  }

  ROS_INFO("Number of supported stream channels %i.", (int) num_streams_);

  // check if every stream channel has been given a channel name
  if (stream_names_.size() < num_streams_) {
    num_streams_ = stream_names_.size();
  }

  // initialize the sensor structs
  for(int i = 0; i < num_streams_; i++) {
    sensors_.push_back(new Sensor());
    p_streams_.push_back(NULL);
    p_buffer_pools_.push_back(CameraBufferPool::Ptr());
    p_camera_info_managers_.push_back(NULL);
    p_camera_info_node_handles_.push_back(NULL);
    camera_infos_.push_back(sensor_msgs::CameraInfoPtr());
    cam_pubs_.push_back(image_transport::CameraPublisher());
    extended_camera_info_pubs_.push_back(ros::Publisher());
  }

  // Get parameter bounds.
  aravis::camera::bounds::get_exposure_time(p_camera_, &config_min_.ExposureTime, &config_max_.ExposureTime);

  aravis::camera::bounds::get_gain(p_camera_, &config_min_.Gain, &config_max_.Gain);

  for(int i = 0; i < num_streams_; i++) {
    if (arv_camera_is_gv_device(p_camera_)) aravis::camera::gv::select_stream_channel(p_camera_,i);

    aravis::camera::get_sensor_size(p_camera_, &sensors_[i]->width, &sensors_[i]->height);
  }

  aravis::camera::bounds::get_width(p_camera_, &roi_.width_min, &roi_.width_max);
  aravis::camera::bounds::get_height(p_camera_, &roi_.height_min, &roi_.height_max);

  aravis::camera::bounds::get_frame_rate(p_camera_, &config_min_.AcquisitionFrameRate, &config_max_.AcquisitionFrameRate);

  if (implemented_features_["FocusPos"])
  {
    gint64 focus_min64, focus_max64;
    aravis::device::feature::bounds::get_integer(p_device_, "FocusPos", &focus_min64, &focus_max64);
    config_min_.FocusPos = focus_min64;
    config_max_.FocusPos = focus_max64;
  }
  else
  {
    config_min_.FocusPos = 0;
    config_max_.FocusPos = 0;
  }

#if ARAVIS_HAS_USB_MODE
  ArvUvUsbMode usb_mode = ARV_UV_USB_MODE_DEFAULT;
  // ArvUvUsbMode mode = ARV_UV_USB_MODE_SYNC;
  // ArvUvUsbMode mode = ARV_UV_USB_MODE_ASYNC;
  std::string usb_mode_arg = "default";
  if (pnh.getParam("usb_mode", usb_mode_arg)) {
    if (usb_mode_arg.size() > 0) {
      if (usb_mode_arg[0] == 's' or usb_mode_arg[0] == 'S') { usb_mode = ARV_UV_USB_MODE_SYNC; }
      else if (usb_mode_arg[0] == 'a' or usb_mode_arg[0] == 'A') { usb_mode = ARV_UV_USB_MODE_ASYNC; }
      else if (usb_mode_arg[0] == 'd' or usb_mode_arg[0] == 'D') { usb_mode = ARV_UV_USB_MODE_DEFAULT; }
      else {
          ROS_WARN_STREAM("Unrecognized USB mode "
                          << usb_mode_arg << " (recognized modes: SYNC, ASYNC and DEFAULT), using DEFAULT ...");
      }
    } else {
          ROS_WARN("Empty USB mode (recognized modes: SYNC, ASYNC and DEFAULT), using DEFAULT ...");
    }
  }
  if (arv_camera_is_uv_device(p_camera_)) arv_uv_device_set_usb_mode(ARV_UV_DEVICE(p_device_), usb_mode);
#endif

  for(int i = 0; i < num_streams_; i++) {
    if (arv_camera_is_gv_device(p_camera_)) aravis::camera::gv::select_stream_channel(p_camera_, i);

    // Initial camera settings.
    if (implemented_features_["ExposureTime"]){
      aravis::camera::set_exposure_time(p_camera_, config_.ExposureTime);
    } else if (implemented_features_["ExposureTimeAbs"]) {
      aravis::device::feature::set_float(p_device_, "ExposureTimeAbs", config_.ExposureTime);
    }

    if (implemented_features_["Gain"]) {
      aravis::camera::set_gain(p_camera_, config_.Gain);
    }

    if (implemented_features_["AcquisitionFrameRateEnable"]) {
      aravis::device::feature::set_integer(p_device_, "AcquisitionFrameRateEnable", 1);
    }
    if (implemented_features_["AcquisitionFrameRate"]) {
      aravis::camera::set_frame_rate(p_camera_, config_.AcquisitionFrameRate);
    }

    // init default to full sensor resolution
    aravis::camera::set_region(p_camera_, 0, 0, roi_.width_max, roi_.height_max);

    // Set up the triggering.
    if (implemented_features_["TriggerMode"] && implemented_features_["TriggerSelector"])
    {
      aravis::device::feature::set_string(p_device_, "TriggerSelector", "FrameStart");
      aravis::device::feature::set_string(p_device_, "TriggerMode", "Off");
    }

    // possibly set or override from given parameter
    writeCameraFeaturesFromRosparam();
  }

  // get current state of camera for config_
  aravis::camera::get_region(p_camera_, &roi_.x, &roi_.y, &roi_.width, &roi_.height);
  config_.AcquisitionMode =
      implemented_features_["AcquisitionMode"] ? aravis::device::feature::get_string(p_device_, "AcquisitionMode") :
          "Continuous";
  config_.AcquisitionFrameRate =
      implemented_features_["AcquisitionFrameRate"] ? aravis::camera::get_frame_rate(p_camera_) : 0.0;
  config_.ExposureAuto =
      implemented_features_["ExposureAuto"] ? aravis::device::feature::get_string(p_device_, "ExposureAuto") : "Off";
  config_.ExposureTime = implemented_features_["ExposureTime"] ? aravis::camera::get_exposure_time(p_camera_) : 0.0;
  config_.GainAuto =
      implemented_features_["GainAuto"] ? aravis::device::feature::get_string(p_device_, "GainAuto") : "Off";
  config_.Gain = implemented_features_["Gain"] ? aravis::camera::get_gain(p_camera_) : 0.0;
  config_.TriggerMode =
      implemented_features_["TriggerMode"] ? aravis::device::feature::get_string(p_device_, "TriggerMode") : "Off";
  config_.TriggerSource =
      implemented_features_["TriggerSource"] ? aravis::device::feature::get_string(p_device_, "TriggerSource") :
          "Software";

  // get pixel format name and translate into corresponding ROS name
  for(int i = 0; i < num_streams_; i++) {
    if (arv_camera_is_gv_device(p_camera_)) aravis::camera::gv::select_stream_channel(p_camera_,i);

    std::string source_selector = "Source" + std::to_string(i);

    if (implemented_features_["SourceSelector"])
        aravis::device::feature::set_string(p_device_, "SourceSelector", source_selector.c_str());
    if (implemented_features_["PixelFormat"] && pixel_formats[i].size())
        aravis::device::feature::set_string(p_device_, "PixelFormat", pixel_formats[i].c_str());

    if (implemented_features_["PixelFormat"])
      sensors_[i]->pixel_format = std::string(aravis::device::feature::get_string(p_device_, "PixelFormat"));
    const auto sensor_iter = CONVERSIONS_DICTIONARY.find(sensors_[i]->pixel_format);
    if (sensor_iter!=CONVERSIONS_DICTIONARY.end()) {
      convert_formats.push_back(sensor_iter->second);
    }
    else {
      ROS_WARN_STREAM("There is no known conversion from " << sensors_[i]->pixel_format << " to a usual ROS image encoding. Likely you need to implement one.");
    }

    if (implemented_features_["PixelFormat"])
      sensors_[i]->n_bits_pixel = ARV_PIXEL_FORMAT_BIT_PER_PIXEL(
          aravis::device::feature::get_integer(p_device_, "PixelFormat"));
    config_.FocusPos =
        implemented_features_["FocusPos"] ? aravis::device::feature::get_integer(p_device_, "FocusPos") : 0;

  }

  // Get other (non GenIcam) parameter current values.
  pnh.param<double>("softwaretriggerrate", config_.softwaretriggerrate, config_.softwaretriggerrate);
  pnh.param<int>("mtu", config_.mtu, config_.mtu);
  pnh.param<std::string>("frame_id", config_.frame_id, config_.frame_id);
  pnh.param<bool>("auto_master", config_.AutoMaster, config_.AutoMaster);
  pnh.param<bool>("auto_slave", config_.AutoSlave, config_.AutoSlave);
  setAutoMaster(config_.AutoMaster);
  setAutoSlave(config_.AutoSlave);

  if (pub_tf_optical_)
  {
    tf_optical_.header.frame_id = config_.frame_id;
    tf_optical_.child_frame_id = config_.frame_id + "_optical";
    tf_optical_.transform.translation.x = 0.0;
    tf_optical_.transform.translation.y = 0.0;
    tf_optical_.transform.translation.z = 0.0;
    tf_optical_.transform.rotation.x = -0.5;
    tf_optical_.transform.rotation.y = 0.5;
    tf_optical_.transform.rotation.z = -0.5;
    tf_optical_.transform.rotation.w = 0.5;

    double tf_publish_rate;
    pnh.param<double>("tf_publish_rate", tf_publish_rate, 0);
    if (tf_publish_rate > 0.)
    {
      // publish dynamic tf at given rate (recommended when running as a Nodelet, since latching has bugs-by-design)
      p_tb_.reset(new tf2_ros::TransformBroadcaster());
      tf_dyn_thread_ = std::thread(&CameraAravisNodelet::publishTfLoop, this, tf_publish_rate);
    }
    else
    {
      // publish static tf only once (latched)
      p_stb_.reset(new tf2_ros::StaticTransformBroadcaster());
      tf_optical_.header.stamp = ros::Time::now();
      p_stb_->sendTransform(tf_optical_);
    }
  }

  // default calibration url is [DeviceSerialNumber/DeviceID].yaml
  if(calib_urls[0].empty()) {
    ArvGcNode *p_gc_node = arv_device_get_feature(p_device_, "DeviceSerialNumber");

    GuardedGError error;
    bool is_implemented = arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(p_gc_node), error.storeError());
    LOG_GERROR_ARAVIS(error);

    if(is_implemented) {

      // If the feature DeviceSerialNumber is not string, it indicates that the camera is using an older version of the genicam SFNC.
      // Older camera models do not have a DeviceSerialNumber as string, but as integer and often set to 0.
      // In those cases use the outdated DeviceID (deprecated since genicam SFNC v2.0).

      if (G_TYPE_CHECK_INSTANCE_TYPE(p_gc_node, G_TYPE_STRING)) {
        calib_urls[0] = aravis::device::feature::get_string(p_device_, "DeviceSerialNumber");
        calib_urls[0] += ".yaml";
      } else if (G_TYPE_CHECK_INSTANCE_TYPE(p_gc_node, G_TYPE_INT64)) {
        calib_urls[0] = aravis::device::feature::get_string(p_device_, "DeviceID");
        calib_urls[0] += ".yaml";
      }
    }
  }

  for(int i = 0; i < num_streams_; i++) {
    // Start the camerainfo manager.
    std::string camera_info_frame_id = config_.frame_id;
    if(!stream_names_[i].empty())
      camera_info_frame_id = config_.frame_id + '/' + stream_names_[i];

    // Use separate node handles for CameraInfoManagers when using a Multisource Camera
    if(!stream_names_[i].empty()) {
      p_camera_info_node_handles_[i].reset(new ros::NodeHandle(pnh, stream_names_[i]));
      p_camera_info_managers_[i].reset(new camera_info_manager::CameraInfoManager(*p_camera_info_node_handles_[i], camera_info_frame_id, calib_urls[i]));
    } else {
      p_camera_info_managers_[i].reset(new camera_info_manager::CameraInfoManager(pnh, camera_info_frame_id, calib_urls[i]));
    }


    ROS_INFO("Reset %s Camera Info Manager", stream_names_[i].c_str());
    ROS_INFO("%s Calib URL: %s", stream_names_[i].c_str(), calib_urls[i].c_str());

    // publish an ExtendedCameraInfo message
    setExtendedCameraInfo(stream_names_[i], i);
  }

  // update the reconfigure config
  reconfigure_server_->setConfigMin(config_min_);
  reconfigure_server_->setConfigMax(config_max_);
  reconfigure_server_->updateConfig(config_);
  ros::Duration(2.0).sleep();
  reconfigure_server_->setCallback(boost::bind(&CameraAravisNodelet::rosReconfigureCallback, this, _1, _2));

  // Print information.
  ROS_INFO("    Using Camera Configuration:");
  ROS_INFO("    ---------------------------");
  ROS_INFO("    Vendor name          = %s", aravis::device::feature::get_string(p_device_, "DeviceVendorName"));
  ROS_INFO("    Model name           = %s", aravis::device::feature::get_string(p_device_, "DeviceModelName"));
  ROS_INFO("    Device id            = %s", aravis::device::feature::get_string(p_device_, "DeviceUserID"));
  ROS_INFO("    Serial number        = %s", aravis::device::feature::get_string(p_device_, "DeviceSerialNumber"));
  ROS_INFO(
      "    Type                 = %s",
      arv_camera_is_uv_device(p_camera_) ? "USB3Vision" :
          (arv_camera_is_gv_device(p_camera_) ? "GigEVision" : "Other"));
  ROS_INFO("    Sensor width         = %d", sensors_[0]->width);
  ROS_INFO("    Sensor height        = %d", sensors_[0]->height);
  ROS_INFO("    ROI x,y,w,h          = %d, %d, %d, %d", roi_.x, roi_.y, roi_.width, roi_.height);
  ROS_INFO("    Pixel format         = %s", sensors_[0]->pixel_format.c_str());
  ROS_INFO("    BitsPerPixel         = %lu", sensors_[0]->n_bits_pixel);
  ROS_INFO(
      "    Acquisition Mode     = %s",
      implemented_features_["AcquisitionMode"] ? aravis::device::feature::get_string(p_device_, "AcquisitionMode") :
          "(not implemented in camera)");
  ROS_INFO(
      "    Trigger Mode         = %s",
      implemented_features_["TriggerMode"] ? aravis::device::feature::get_string(p_device_, "TriggerMode") :
          "(not implemented in camera)");
  ROS_INFO(
      "    Trigger Source       = %s",
      implemented_features_["TriggerSource"] ? aravis::device::feature::get_string(p_device_, "TriggerSource") :
          "(not implemented in camera)");
  ROS_INFO("    Can set FrameRate:     %s", implemented_features_["AcquisitionFrameRate"] ? "True" : "False");
  if (implemented_features_["AcquisitionFrameRate"])
  {
    ROS_INFO("    AcquisitionFrameRate = %g hz", config_.AcquisitionFrameRate);
  }

  ROS_INFO("    Can set Exposure:      %s", implemented_features_["ExposureTime"] ? "True" : "False");
  if (implemented_features_["ExposureTime"])
  {
    ROS_INFO("    Can set ExposureAuto:  %s", implemented_features_["ExposureAuto"] ? "True" : "False");
    ROS_INFO("    Exposure             = %g us in range [%g,%g]", config_.ExposureTime, config_min_.ExposureTime,
             config_max_.ExposureTime);
  }

  ROS_INFO("    Can set Gain:          %s", implemented_features_["Gain"] ? "True" : "False");
  if (implemented_features_["Gain"])
  {
    ROS_INFO("    Can set GainAuto:      %s", implemented_features_["GainAuto"] ? "True" : "False");
    ROS_INFO("    Gain                 = %f %% in range [%f,%f]", config_.Gain, config_min_.Gain, config_max_.Gain);
  }

  ROS_INFO("    Can set FocusPos:      %s", implemented_features_["FocusPos"] ? "True" : "False");

  if (implemented_features_["GevSCPSPacketSize"])
    ROS_INFO("    Network mtu          = %lu", aravis::device::feature::get_integer(p_device_, "GevSCPSPacketSize"));

  ROS_INFO("    ---------------------------");

  // Reset PTP clock
  if (use_ptp_stamp_)
    resetPtpClock();

  // spawn camera stream in thread, so onInit() is not blocked
  spawning_ = true;
  spawn_stream_thread_ = std::thread(&CameraAravisNodelet::spawnStream, this);
}

void CameraAravisNodelet::spawnStream()
{
  ros::NodeHandle nh  = getNodeHandle();
  ros::NodeHandle pnh = getPrivateNodeHandle();
  GuardedGError error;

  for(int i = 0; i < num_streams_; i++) {
    while (spawning_) {
      if (arv_camera_is_gv_device(p_camera_)) aravis::camera::gv::select_stream_channel(p_camera_, i);

      p_streams_[i] = aravis::camera::create_stream(p_camera_, NULL, NULL);

      if (p_streams_[i])
      {
        // Load up some buffers.
        if (arv_camera_is_gv_device(p_camera_)) aravis::camera::gv::select_stream_channel(p_camera_, i);

        const gint n_bytes_payload_stream_ = aravis::camera::get_payload(p_camera_);

        p_buffer_pools_[i].reset(new CameraBufferPool(p_streams_[i], n_bytes_payload_stream_, 10));

        if (arv_camera_is_gv_device(p_camera_))
        {
          tuneGvStream(reinterpret_cast<ArvGvStream*>(p_streams_[i]));
        }
        break;
      }
      else
      {
        ROS_WARN("Stream %i: Could not create image stream for %s.  Retrying...", i, guid_.c_str());
        ros::Duration(1.0).sleep();
        ros::spinOnce();
      }
    }
  }

  // Monitor whether anyone is subscribed to the camera stream
  std::vector<image_transport::SubscriberStatusCallback> image_cbs_;
  std::vector<ros::SubscriberStatusCallback> info_cbs_;

  image_transport::SubscriberStatusCallback image_cb = [this](const image_transport::SingleSubscriberPublisher &ssp)
  { this->rosConnectCallback();};
  ros::SubscriberStatusCallback info_cb = [this](const ros::SingleSubscriberPublisher &ssp)
  { this->rosConnectCallback();};

  for(int i = 0; i < num_streams_; i++) {
    image_transport::ImageTransport *p_transport;
    // Set up image_raw
    std::string topic_name = this->getName();
    p_transport = new image_transport::ImageTransport(pnh);
    if(num_streams_ != 1 || !stream_names_[i].empty()) {
      topic_name += "/" + stream_names_[i];
    }

    cam_pubs_[i] = p_transport->advertiseCamera(
      ros::names::remap(topic_name + "/image_raw"),
      1, image_cb, image_cb, info_cb, info_cb);
  }

  // Connect signals with callbacks.
  for(int i = 0; i < num_streams_; i++) {
    StreamIdData* data = new StreamIdData();
    data->can = this;
    data->stream_id = i;
    g_signal_connect(p_streams_[i], "new-buffer", (GCallback)CameraAravisNodelet::newBufferReadyCallback, data);
  }
  g_signal_connect(p_device_, "control-lost", (GCallback)CameraAravisNodelet::controlLostCallback, this);

  for(int i = 0; i < num_streams_; i++) {
    arv_stream_set_emit_signals(p_streams_[i], TRUE);
  }

  if (std::any_of(cam_pubs_.begin(), cam_pubs_.end(),
    [](image_transport::CameraPublisher pub){ return pub.getNumSubscribers() > 0; })
  ) {
    aravis::camera::start_acquisition(p_camera_);
  }

  this->get_integer_service_ = pnh.advertiseService("get_integer_feature_value", &CameraAravisNodelet::getIntegerFeatureCallback, this);
  this->get_float_service_ = pnh.advertiseService("get_float_feature_value", &CameraAravisNodelet::getFloatFeatureCallback, this);
  this->get_string_service_ = pnh.advertiseService("get_string_feature_value", &CameraAravisNodelet::getStringFeatureCallback, this);
  this->get_boolean_service_ = pnh.advertiseService("get_boolean_feature_value", &CameraAravisNodelet::getBooleanFeatureCallback, this);

  this->set_integer_service_ = pnh.advertiseService("set_integer_feature_value", &CameraAravisNodelet::setIntegerFeatureCallback, this);
  this->set_float_service_ = pnh.advertiseService("set_float_feature_value", &CameraAravisNodelet::setFloatFeatureCallback, this);
  this->set_string_service_ = pnh.advertiseService("set_string_feature_value", &CameraAravisNodelet::setStringFeatureCallback, this);
  this->set_boolean_service_ = pnh.advertiseService("set_boolean_feature_value", &CameraAravisNodelet::setBooleanFeatureCallback, this);

  this->exec_command_service_ = pnh.advertiseService("execute_command", &CameraAravisNodelet::executeCommandCallback, this);

  ROS_INFO("Done initializing camera_aravis.");
}

bool CameraAravisNodelet::getIntegerFeatureCallback(camera_aravis::get_integer_feature_value::Request& request, camera_aravis::get_integer_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  response.response = arv_device_get_integer_feature_value(this->p_device_, feature_name, error.storeError());
  LOG_GERROR_ARAVIS(error);
  return !error;
}

bool CameraAravisNodelet::setIntegerFeatureCallback(camera_aravis::set_integer_feature_value::Request& request, camera_aravis::set_integer_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  guint64 value = request.value;
  arv_device_set_integer_feature_value(this->p_device_, feature_name, value, error.storeError());
  LOG_GERROR_ARAVIS(error);
  response.ok = !error;
  return true;
}

bool CameraAravisNodelet::getFloatFeatureCallback(camera_aravis::get_float_feature_value::Request& request, camera_aravis::get_float_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  response.response = arv_device_get_float_feature_value(this->p_device_, feature_name, error.storeError());
  LOG_GERROR_ARAVIS(error);
  return !error;
}

bool CameraAravisNodelet::setFloatFeatureCallback(camera_aravis::set_float_feature_value::Request& request, camera_aravis::set_float_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  const double value = request.value;
  arv_device_set_float_feature_value(this->p_device_, feature_name, value, error.storeError());
  LOG_GERROR_ARAVIS(error);
  response.ok = !error;
  return true;
}

bool CameraAravisNodelet::getStringFeatureCallback(camera_aravis::get_string_feature_value::Request& request, camera_aravis::get_string_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  response.response = arv_device_get_string_feature_value(this->p_device_, feature_name, error.storeError());
  LOG_GERROR_ARAVIS(error);
  return !error;
}

bool CameraAravisNodelet::setStringFeatureCallback(camera_aravis::set_string_feature_value::Request& request, camera_aravis::set_string_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  const char* value = request.value.c_str();
  arv_device_set_string_feature_value(this->p_device_, feature_name, value, error.storeError());
  LOG_GERROR_ARAVIS(error);
  response.ok = !error;
  return true;
}

bool CameraAravisNodelet::getBooleanFeatureCallback(camera_aravis::get_boolean_feature_value::Request& request, camera_aravis::get_boolean_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  response.response = arv_device_get_boolean_feature_value(this->p_device_, feature_name, error.storeError());
  LOG_GERROR_ARAVIS(error);
  return !error;
}

bool CameraAravisNodelet::setBooleanFeatureCallback(camera_aravis::set_boolean_feature_value::Request& request, camera_aravis::set_boolean_feature_value::Response& response)
{
  GuardedGError error;
  const char* feature_name = request.feature.c_str();
  const bool value = request.value;
  arv_device_set_boolean_feature_value(this->p_device_, feature_name, value, error.storeError());
  LOG_GERROR_ARAVIS(error);
  response.ok = !error;
  return true;
}

bool CameraAravisNodelet::executeCommandCallback(camera_aravis::execute_command::Request& request, camera_aravis::execute_command::Response& response) {
  GuardedGError error;
  const char* command = request.command.c_str();
  arv_device_execute_command(this->p_device_, command, error.storeError());
  LOG_GERROR_ARAVIS(error);
  response.response = !error;
  return true;
}

void CameraAravisNodelet::resetPtpClock()
{
  // a PTP slave can take the following states: Slave, Listening, Uncalibrated, Faulty, Disabled
  std::string ptp_status(aravis::device::feature::get_string(p_device_, "GevIEEE1588Status"));
  if (ptp_status == std::string("Faulty") || ptp_status == std::string("Disabled"))
  {
    ROS_INFO("camera_aravis: Reset ptp clock (was set to %s)", ptp_status.c_str());
    aravis::device::feature::set_boolean(p_device_, "GevIEEE1588", false);
    aravis::device::feature::set_boolean(p_device_, "GevIEEE1588", true);
  }
  
}

void CameraAravisNodelet::cameraAutoInfoCallback(const CameraAutoInfoConstPtr &msg_ptr)
{
  if (config_.AutoSlave && p_device_)
  {

    if (auto_params_.exposure_time != msg_ptr->exposure_time && implemented_features_["ExposureTime"])
    {
      aravis::device::feature::set_float(p_device_, "ExposureTime", msg_ptr->exposure_time);
    }

    if (implemented_features_["Gain"])
    {
      if (auto_params_.gain != msg_ptr->gain)
      {
        if (implemented_features_["GainSelector"])
        {
          aravis::device::feature::set_string(p_device_, "GainSelector", "All");
        }
        aravis::device::feature::set_float(p_device_, "Gain", msg_ptr->gain);
      }

      if (implemented_features_["GainSelector"])
      {
        if (auto_params_.gain_red != msg_ptr->gain_red)
        {
          aravis::device::feature::set_string(p_device_, "GainSelector", "Red");
          aravis::device::feature::set_float(p_device_, "Gain", msg_ptr->gain_red);
        }

        if (auto_params_.gain_green != msg_ptr->gain_green)
        {
          aravis::device::feature::set_string(p_device_, "GainSelector", "Green");
          aravis::device::feature::set_float(p_device_, "Gain", msg_ptr->gain_green);
        }

        if (auto_params_.gain_blue != msg_ptr->gain_blue)
        {
          aravis::device::feature::set_string(p_device_, "GainSelector", "Blue");
          aravis::device::feature::set_float(p_device_, "Gain", msg_ptr->gain_blue);
        }
      }
    }

    if (implemented_features_["BlackLevel"])
    {
      if (auto_params_.black_level != msg_ptr->black_level)
      {
        if (implemented_features_["BlackLevelSelector"])
        {
          aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "All");
        }
        aravis::device::feature::set_float(p_device_, "BlackLevel", msg_ptr->black_level);
      }

      if (implemented_features_["BlackLevelSelector"])
      {
        if (auto_params_.bl_red != msg_ptr->bl_red)
        {
          aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Red");
          aravis::device::feature::set_float(p_device_, "BlackLevel", msg_ptr->bl_red);
        }

        if (auto_params_.bl_green != msg_ptr->bl_green)
        {
          aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Green");
          aravis::device::feature::set_float(p_device_, "BlackLevel", msg_ptr->bl_green);
        }

        if (auto_params_.bl_blue != msg_ptr->bl_blue)
        {
          aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Blue");
          aravis::device::feature::set_float(p_device_, "BlackLevel", msg_ptr->bl_blue);
        }
      }
    }

    // White balance as TIS is providing
    if (strcmp("The Imaging Source Europe GmbH", aravis::camera::get_vendor_name(p_camera_)) == 0)
    {
      aravis::device::feature::set_integer(p_device_, "WhiteBalanceRedRegister", (int)(auto_params_.wb_red * 255.));
      aravis::device::feature::set_integer(p_device_, "WhiteBalanceGreenRegister", (int)(auto_params_.wb_green * 255.));
      aravis::device::feature::set_integer(p_device_, "WhiteBalanceBlueRegister", (int)(auto_params_.wb_blue * 255.));
    }
    else if (implemented_features_["BalanceRatio"] && implemented_features_["BalanceRatioSelector"])
    {
      if (auto_params_.wb_red != msg_ptr->wb_red)
      {
        aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Red");
        aravis::device::feature::set_float(p_device_, "BalanceRatio", msg_ptr->wb_red);
      }

      if (auto_params_.wb_green != msg_ptr->wb_green)
      {
        aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Green");
        aravis::device::feature::set_float(p_device_, "BalanceRatio", msg_ptr->wb_green);
      }

      if (auto_params_.wb_blue != msg_ptr->wb_blue)
      {
        aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Blue");
        aravis::device::feature::set_float(p_device_, "BalanceRatio", msg_ptr->wb_blue);
      }
    }

    auto_params_ = *msg_ptr;
  }
}

void CameraAravisNodelet::syncAutoParameters()
{
  auto_params_.exposure_time = auto_params_.gain = auto_params_.gain_red = auto_params_.gain_green =
      auto_params_.gain_blue = auto_params_.black_level = auto_params_.bl_red = auto_params_.bl_green =
          auto_params_.bl_blue = auto_params_.wb_red = auto_params_.wb_green = auto_params_.wb_blue =
              std::numeric_limits<double>::quiet_NaN();

  if (p_device_)
  {
    if (implemented_features_["ExposureTime"])
    {
      auto_params_.exposure_time = aravis::device::feature::get_float(p_device_, "ExposureTime");
    }

    if (implemented_features_["Gain"])
    {
      if (implemented_features_["GainSelector"])
      {
        aravis::device::feature::set_string(p_device_, "GainSelector", "All");
      }
      auto_params_.gain = aravis::device::feature::get_float(p_device_, "Gain");
      if (implemented_features_["GainSelector"])
      {
        aravis::device::feature::set_string(p_device_, "GainSelector", "Red");
        auto_params_.gain_red = aravis::device::feature::get_float(p_device_, "Gain");
        aravis::device::feature::set_string(p_device_, "GainSelector", "Green");
        auto_params_.gain_green = aravis::device::feature::get_float(p_device_, "Gain");
        aravis::device::feature::set_string(p_device_, "GainSelector", "Blue");
        auto_params_.gain_blue = aravis::device::feature::get_float(p_device_, "Gain");
      }
    }

    if (implemented_features_["BlackLevel"])
    {
      if (implemented_features_["BlackLevelSelector"])
      {
        aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "All");
      }
      auto_params_.black_level = aravis::device::feature::get_float(p_device_, "BlackLevel");
      if (implemented_features_["BlackLevelSelector"])
      {
        aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Red");
        auto_params_.bl_red = aravis::device::feature::get_float(p_device_, "BlackLevel");
        aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Green");
        auto_params_.bl_green = aravis::device::feature::get_float(p_device_, "BlackLevel");
        aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "Blue");
        auto_params_.bl_blue = aravis::device::feature::get_float(p_device_, "BlackLevel");
      }
    }

    // White balance as TIS is providing
    if (strcmp("The Imaging Source Europe GmbH", aravis::camera::get_vendor_name(p_camera_)) == 0)
    {
      auto_params_.wb_red = aravis::device::feature::get_integer(p_device_, "WhiteBalanceRedRegister") / 255.;
      auto_params_.wb_green = aravis::device::feature::get_integer(p_device_, "WhiteBalanceGreenRegister") / 255.;
      auto_params_.wb_blue = aravis::device::feature::get_integer(p_device_, "WhiteBalanceBlueRegister") / 255.;
    }
    // the standard way
    else if (implemented_features_["BalanceRatio"] && implemented_features_["BalanceRatioSelector"])
    {
      aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Red");
      auto_params_.wb_red = aravis::device::feature::get_float(p_device_, "BalanceRatio");
      aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Green");
      auto_params_.wb_green = aravis::device::feature::get_float(p_device_, "BalanceRatio");
      aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Blue");
      auto_params_.wb_blue = aravis::device::feature::get_float(p_device_, "BalanceRatio");
    }
  }
}

void CameraAravisNodelet::setAutoMaster(bool value)
{
  if (value)
  {
    syncAutoParameters();
    auto_pub_ = getNodeHandle().advertise<CameraAutoInfo>(ros::names::remap("camera_auto_info"), 1, true);
  }
  else
  {
    auto_pub_.shutdown();
  }
}

void CameraAravisNodelet::setAutoSlave(bool value)
{
  if (value)
  {
    // deactivate all auto functions
    if (implemented_features_["ExposureAuto"])
    {
      aravis::device::feature::set_string(p_device_, "ExposureAuto", "Off");
    }
    if (implemented_features_["GainAuto"])
    {
      aravis::device::feature::set_string(p_device_, "GainAuto", "Off");
    }
    if (implemented_features_["GainAutoBalance"])
    {
      aravis::device::feature::set_string(p_device_, "GainAutoBalance", "Off");
    }
    if (implemented_features_["BlackLevelAuto"])
    {
      aravis::device::feature::set_string(p_device_, "BlackLevelAuto", "Off");
    }
    if (implemented_features_["BlackLevelAutoBalance"])
    {
      aravis::device::feature::set_string(p_device_, "BlackLevelAutoBalance", "Off");
    }
    if (implemented_features_["BalanceWhiteAuto"])
    {
      aravis::device::feature::set_string(p_device_, "BalanceWhiteAuto", "Off");
    }
    syncAutoParameters();
    auto_sub_ = getNodeHandle().subscribe(ros::names::remap("camera_auto_info"), 1,
                                          &CameraAravisNodelet::cameraAutoInfoCallback, this);
  }
  else
  {
    auto_sub_.shutdown();
  }
}

void CameraAravisNodelet::setExtendedCameraInfo(std::string channel_name, size_t stream_id)
{
  if (pub_ext_camera_info_)
  {
    if (channel_name.empty()) {
      extended_camera_info_pubs_[stream_id]  = getNodeHandle().advertise<ExtendedCameraInfo>(ros::names::remap("extended_camera_info"), 1, true);
    } else {
      extended_camera_info_pubs_[stream_id]  = getNodeHandle().advertise<ExtendedCameraInfo>(ros::names::remap(channel_name + "/extended_camera_info"), 1, true);
    }
  }
  else
  {
    extended_camera_info_pubs_[stream_id].shutdown();
  }
}

// Extra stream options for GigEVision streams.
void CameraAravisNodelet::tuneGvStream(ArvGvStream *p_stream)
{
  gboolean b_auto_buffer = FALSE;
  gboolean b_packet_resend = TRUE;
  unsigned int timeout_packet = 40; // milliseconds
  unsigned int timeout_frame_retention = 200;

  if (p_stream)
  {
    if (!ARV_IS_GV_STREAM(p_stream))
    {
      ROS_WARN("Stream is not a GV_STREAM");
      return;
    }

    if (b_auto_buffer)
      g_object_set(p_stream, "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO, "socket-buffer-size", 0,
      NULL);
    if (!b_packet_resend)
      g_object_set(p_stream, "packet-resend",
                   b_packet_resend ? ARV_GV_STREAM_PACKET_RESEND_ALWAYS : ARV_GV_STREAM_PACKET_RESEND_NEVER,
                   NULL);
    g_object_set(p_stream, "packet-timeout", timeout_packet * 1000, "frame-retention", timeout_frame_retention * 1000,
    NULL);
  }
}

void CameraAravisNodelet::rosReconfigureCallback(Config &config, uint32_t level)
{
  reconfigure_mutex_.lock();
  std::string tf_prefix = tf::getPrefixParam(getNodeHandle());
  ROS_DEBUG_STREAM("tf_prefix: " << tf_prefix);

  if (config.frame_id == "")
    config.frame_id = this->getName();

  // Limit params to legal values.
  config.AcquisitionFrameRate = CLAMP(config.AcquisitionFrameRate, config_min_.AcquisitionFrameRate,
                                      config_max_.AcquisitionFrameRate);
  config.ExposureTime = CLAMP(config.ExposureTime, config_min_.ExposureTime, config_max_.ExposureTime);
  config.Gain = CLAMP(config.Gain, config_min_.Gain, config_max_.Gain);
  config.FocusPos = CLAMP(config.FocusPos, config_min_.FocusPos, config_max_.FocusPos);
  config.frame_id = tf::resolve(tf_prefix, config.frame_id);

  if (use_ptp_stamp_)
    resetPtpClock();

  // stop auto functions if slave
  if (config.AutoSlave)
  {
    config.ExposureAuto = "Off";
    config.GainAuto = "Off";
    // todo: all other auto functions "Off"
  }

  // reset values controlled by auto functions
  if (config.ExposureAuto.compare("Off") != 0)
  {
    config.ExposureTime = config_.ExposureTime;
    ROS_WARN("ExposureAuto is active. Cannot manually set ExposureTime.");
  }
  if (config.GainAuto.compare("Off") != 0)
  {
    config.Gain = config_.Gain;
    ROS_WARN("GainAuto is active. Cannot manually set Gain.");
  }

  // reset FrameRate when triggered
  if (config.TriggerMode.compare("Off") != 0)
  {
    config.AcquisitionFrameRate = config_.AcquisitionFrameRate;
    ROS_WARN("TriggerMode is active (Trigger Source: %s). Cannot manually set AcquisitionFrameRate.", config_.TriggerSource.c_str());
  }

  // Find valid user changes we need to react to.
  const bool changed_auto_master = (config_.AutoMaster != config.AutoMaster);
  const bool changed_auto_slave = (config_.AutoSlave != config.AutoSlave);
  const bool changed_acquisition_frame_rate = (config_.AcquisitionFrameRate != config.AcquisitionFrameRate);
  const bool changed_exposure_auto = (config_.ExposureAuto != config.ExposureAuto);
  const bool changed_exposure_time = (config_.ExposureTime != config.ExposureTime);
  const bool changed_gain_auto = (config_.GainAuto != config.GainAuto);
  const bool changed_gain = (config_.Gain != config.Gain);
  const bool changed_acquisition_mode = (config_.AcquisitionMode != config.AcquisitionMode);
  const bool changed_trigger_mode = (config_.TriggerMode != config.TriggerMode);
  const bool changed_trigger_source = (config_.TriggerSource != config.TriggerSource) || changed_trigger_mode;
  const bool changed_focus_pos = (config_.FocusPos != config.FocusPos);
  const bool changed_mtu = (config_.mtu != config.mtu);

  if (changed_auto_master)
  {
    setAutoMaster(config.AutoMaster);
  }

  if (changed_auto_slave)
  {
    setAutoSlave(config.AutoSlave);
  }

  // Set params into the camera.
  if (changed_exposure_time)
  {
    if (implemented_features_["ExposureTime"])
    {
      ROS_INFO("Set ExposureTime = %f us", config.ExposureTime);
      aravis::camera::set_exposure_time(p_camera_, config.ExposureTime);
    }
    else
      ROS_INFO("Camera does not support ExposureTime.");
  }

  if (changed_gain)
  {
    if (implemented_features_["Gain"])
    {
      ROS_INFO("Set gain = %f", config.Gain);
      aravis::camera::set_gain(p_camera_, config.Gain);
    }
    else
      ROS_INFO("Camera does not support Gain or GainRaw.");
  }

  if (changed_exposure_auto)
  {
    if (implemented_features_["ExposureAuto"] && implemented_features_["ExposureTime"])
    {
      ROS_INFO("Set ExposureAuto = %s", config.ExposureAuto.c_str());
      aravis::device::feature::set_string(p_device_, "ExposureAuto", config.ExposureAuto.c_str());
      if (config.ExposureAuto.compare("Once") == 0)
      {
        ros::Duration(2.0).sleep();
        config.ExposureTime = aravis::camera::get_exposure_time(p_camera_);
        ROS_INFO("Get ExposureTime = %f us", config.ExposureTime);
        config.ExposureAuto = "Off";
      }
    }
    else
      ROS_INFO("Camera does not support ExposureAuto.");
  }
  if (changed_gain_auto)
  {
    if (implemented_features_["GainAuto"] && implemented_features_["Gain"])
    {
      ROS_INFO("Set GainAuto = %s", config.GainAuto.c_str());
      aravis::device::feature::set_string(p_device_, "GainAuto", config.GainAuto.c_str());
      if (config.GainAuto.compare("Once") == 0)
      {
        ros::Duration(2.0).sleep();
        config.Gain = aravis::camera::get_gain(p_camera_);
        ROS_INFO("Get Gain = %f", config.Gain);
        config.GainAuto = "Off";
      }
    }
    else
      ROS_INFO("Camera does not support GainAuto.");
  }

  if (changed_acquisition_frame_rate)
  {
    if (implemented_features_["AcquisitionFrameRate"])
    {
      ROS_INFO("Set frame rate = %f Hz", config.AcquisitionFrameRate);
      aravis::camera::set_frame_rate(p_camera_, config.AcquisitionFrameRate);
    }
    else
      ROS_INFO("Camera does not support AcquisitionFrameRate.");
  }

  if (changed_trigger_mode)
  {
    if (implemented_features_["TriggerMode"])
    {
      ROS_INFO("Set TriggerMode = %s", config.TriggerMode.c_str());
      aravis::device::feature::set_string(p_device_, "TriggerMode", config.TriggerMode.c_str());
    }
    else
      ROS_INFO("Camera does not support TriggerMode.");
  }

  if (changed_trigger_source)
  {
    // delete old software trigger thread if active
    software_trigger_active_ = false;
    if (software_trigger_thread_.joinable())
    {
      software_trigger_thread_.join();
    }

    if (implemented_features_["TriggerSource"])
    {
      ROS_INFO("Set TriggerSource = %s", config.TriggerSource.c_str());
      aravis::device::feature::set_string(p_device_, "TriggerSource", config.TriggerSource.c_str());
    }
    else
    {
      ROS_INFO("Camera does not support TriggerSource.");
    }

    // activate on demand
    if (config.TriggerMode.compare("On") == 0 && config.TriggerSource.compare("Software") == 0)
    {
      if (implemented_features_["TriggerSoftware"])
      {
        config_.softwaretriggerrate = config.softwaretriggerrate;
        ROS_INFO("Set softwaretriggerrate = %f", 1000.0 / ceil(1000.0 / config.softwaretriggerrate));

        // Turn on software timer callback.
        software_trigger_thread_ = std::thread(&CameraAravisNodelet::softwareTriggerLoop, this);
      }
      else
      {
        ROS_INFO("Camera does not support TriggerSoftware command.");
      }
    }
  }

  if (changed_focus_pos)
  {
    if (implemented_features_["FocusPos"])
    {
      ROS_INFO("Set FocusPos = %d", config.FocusPos);
      aravis::device::feature::set_integer(p_device_, "FocusPos", config.FocusPos);
      ros::Duration(1.0).sleep();
      config.FocusPos = aravis::device::feature::get_integer(p_device_, "FocusPos");
      ROS_INFO("Get FocusPos = %d", config.FocusPos);
    }
    else
      ROS_INFO("Camera does not support FocusPos.");
  }

  if (changed_mtu)
  {
    if (implemented_features_["GevSCPSPacketSize"])
    {
      ROS_INFO("Set mtu = %d", config.mtu);
      aravis::device::feature::set_integer(p_device_, "GevSCPSPacketSize", config.mtu);
      ros::Duration(1.0).sleep();
      config.mtu = aravis::device::feature::get_integer(p_device_, "GevSCPSPacketSize");
      ROS_INFO("Get mtu = %d", config.mtu);
    }
    else
      ROS_INFO("Camera does not support mtu (i.e. GevSCPSPacketSize).");
  }

  if (changed_acquisition_mode)
  {
    if (implemented_features_["AcquisitionMode"])
    {
      ROS_INFO("Set AcquisitionMode = %s", config.AcquisitionMode.c_str());
      aravis::device::feature::set_string(p_device_, "AcquisitionMode", config.AcquisitionMode.c_str());

      ROS_INFO("AcquisitionStop");
      aravis::device::execute_command(p_device_, "AcquisitionStop");
      ROS_INFO("AcquisitionStart");
      aravis::device::execute_command(p_device_, "AcquisitionStart");
    }
    else
      ROS_INFO("Camera does not support AcquisitionMode.");
  }

  // adopt new config
  config_ = config;
  reconfigure_mutex_.unlock();
}

void CameraAravisNodelet::rosConnectCallback()
{
  if (p_device_)
  {
    if (std::all_of(cam_pubs_.begin(), cam_pubs_.end(),
      [](image_transport::CameraPublisher pub){ return pub.getNumSubscribers() == 0; })
    ){
      aravis::device::execute_command(p_device_, "AcquisitionStop"); // don't waste CPU if nobody is listening!
    }
    else
    {
      aravis::device::execute_command(p_device_, "AcquisitionStart");
    }
  }
}

void CameraAravisNodelet::newBufferReadyCallback(ArvStream *p_stream, gpointer can_instance)
{

  // workaround to get access to the instance from a static method
  StreamIdData *data = (StreamIdData*) can_instance;
  CameraAravisNodelet *p_can = (CameraAravisNodelet*) data->can;
  size_t stream_id = data->stream_id;
  image_transport::CameraPublisher *p_cam_pub = &p_can->cam_pubs_[stream_id];

  if( p_can->stream_names_[stream_id].empty() )
  {
    newBufferReady(p_stream, p_can, p_can->config_.frame_id, stream_id);
  } else {
    const std::string stream_frame_id = p_can->config_.frame_id + "/" + p_can->stream_names_[stream_id];
    newBufferReady(p_stream, p_can, stream_frame_id, stream_id);
  }

}

void CameraAravisNodelet::newBufferReady(ArvStream *p_stream, CameraAravisNodelet *p_can, std::string frame_id, size_t stream_id)
{
  ArvBuffer *p_buffer = arv_stream_try_pop_buffer(p_stream);

  // check if we risk to drop the next image because of not enough buffers left
  gint n_available_buffers;
  arv_stream_get_n_buffers(p_stream, &n_available_buffers, NULL);
  if (n_available_buffers == 0)
  {
    p_can->p_buffer_pools_[stream_id]->allocateBuffers(1);
  }

  if (p_buffer != NULL)
  {
    if (arv_buffer_get_status(p_buffer) == ARV_BUFFER_STATUS_SUCCESS && p_can->p_buffer_pools_[stream_id]
        && p_can->cam_pubs_[stream_id].getNumSubscribers())
    {
      (p_can->n_buffers_)++;
      // get the image message which wraps around this buffer
      sensor_msgs::ImagePtr msg_ptr = (*(p_can->p_buffer_pools_[stream_id]))[p_buffer];
      // fill the meta information of image message
      // get acquisition time
      guint64 t;
      if (p_can->use_ptp_stamp_)
        t = arv_buffer_get_timestamp(p_buffer);
      else
        t = arv_buffer_get_system_timestamp(p_buffer);
      msg_ptr->header.stamp.fromNSec(t);
      // get frame sequence number
      msg_ptr->header.seq = arv_buffer_get_frame_id(p_buffer);
      // fill other stream properties
      msg_ptr->header.frame_id = frame_id;
      msg_ptr->width = p_can->roi_.width;
      msg_ptr->height = p_can->roi_.height;
      msg_ptr->encoding = p_can->sensors_[stream_id]->pixel_format;
      msg_ptr->step = (msg_ptr->width * p_can->sensors_[stream_id]->n_bits_pixel)/8;

      // do the magic of conversion into a ROS format
      if (p_can->convert_formats[stream_id]) {
        sensor_msgs::ImagePtr cvt_msg_ptr = p_can->p_buffer_pools_[stream_id]->getRecyclableImg();
        p_can->convert_formats[stream_id](msg_ptr, cvt_msg_ptr);
        msg_ptr = cvt_msg_ptr;
      }

      // get current CameraInfo data
      if (!p_can->camera_infos_[stream_id]) {
        p_can->camera_infos_[stream_id].reset(new sensor_msgs::CameraInfo);
      }
      (*p_can->camera_infos_[stream_id]) = p_can->p_camera_info_managers_[stream_id]->getCameraInfo();
      p_can->camera_infos_[stream_id]->header = msg_ptr->header;
      if (p_can->camera_infos_[stream_id]->width == 0 || p_can->camera_infos_[stream_id]->height == 0) {
        ROS_WARN_STREAM_ONCE(
            "The fields image_width and image_height seem not to be set in "
            "the YAML specified by 'camera_info_url' parameter. Please set "
            "them there, because actual image size and specified image size "
            "can be different due to the region of interest (ROI) feature. In "
            "the YAML the image size should be the one on which the camera was "
            "calibrated. See CameraInfo.msg specification!");
        p_can->camera_infos_[stream_id]->width = p_can->roi_.width;
        p_can->camera_infos_[stream_id]->height = p_can->roi_.height;
      }


      p_can->cam_pubs_[stream_id].publish(msg_ptr, p_can->camera_infos_[stream_id]);

      if (p_can->pub_ext_camera_info_) {
        ExtendedCameraInfo extended_camera_info_msg;
        p_can->extended_camera_info_mutex_.lock();

        if (arv_camera_is_gv_device(p_can->p_camera_)) aravis::camera::gv::select_stream_channel(p_can->p_camera_, stream_id);

        extended_camera_info_msg.camera_info = *(p_can->camera_infos_[stream_id]);
        p_can->fillExtendedCameraInfoMessage(extended_camera_info_msg);
        p_can->extended_camera_info_mutex_.unlock();
        p_can->extended_camera_info_pubs_[stream_id].publish(extended_camera_info_msg);
      }

      // check PTP status, camera cannot recover from "Faulty" by itself
      if (p_can->use_ptp_stamp_)
        p_can->resetPtpClock();
    }
    else
    {
      if (arv_buffer_get_status(p_buffer) != ARV_BUFFER_STATUS_SUCCESS) {
        ROS_WARN("(%s) Frame error: %s", frame_id.c_str(), szBufferStatusFromInt[arv_buffer_get_status(p_buffer)]);
      }
      arv_stream_push_buffer(p_stream, p_buffer);
    }
  }

  // publish current lighting settings if this camera is configured as master
  if (p_can->config_.AutoMaster)
  {
    p_can->syncAutoParameters();
    p_can->auto_pub_.publish(p_can->auto_params_);
  }
}

void CameraAravisNodelet::fillExtendedCameraInfoMessage(ExtendedCameraInfo &msg)
{
  const char *vendor_name = aravis::camera::get_vendor_name(p_camera_);

  if (strcmp("Basler", vendor_name) == 0) {
    msg.exposure_time = aravis::device::feature::get_float(p_device_, "ExposureTimeAbs");
  }
  else if (implemented_features_["ExposureTime"])
  {
    msg.exposure_time = aravis::device::feature::get_float(p_device_, "ExposureTime");
  }

  if (strcmp("Basler", vendor_name) == 0) {
    msg.gain = static_cast<float>(aravis::device::feature::get_integer(p_device_, "GainRaw"));
  }
  else if (implemented_features_["Gain"])
  {
    msg.gain = aravis::device::feature::get_float(p_device_, "Gain");
  }
  if (strcmp("Basler", vendor_name) == 0) {
    aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "All");
    msg.black_level = static_cast<float>(aravis::device::feature::get_integer(p_device_, "BlackLevelRaw"));
  } else if (strcmp("JAI Corporation", vendor_name) == 0) {
    // Reading the black level register for both streams of the JAI FS 3500D takes too long.
    // The frame rate the drops below 10 fps.
    msg.black_level = 0;
  } else {
    aravis::device::feature::set_string(p_device_, "BlackLevelSelector", "All");
    msg.black_level = aravis::device::feature::get_float(p_device_, "BlackLevel");
  }

  // White balance as TIS is providing
  if (strcmp("The Imaging Source Europe GmbH", vendor_name) == 0)
  {
    msg.white_balance_red = aravis::device::feature::get_integer(p_device_, "WhiteBalanceRedRegister") / 255.;
    msg.white_balance_green = aravis::device::feature::get_integer(p_device_, "WhiteBalanceGreenRegister") / 255.;
    msg.white_balance_blue = aravis::device::feature::get_integer(p_device_, "WhiteBalanceBlueRegister") / 255.;
  }
  // the JAI cameras become too slow when reading out the DigitalRed and DigitalBlue values
  // the white balance is adjusted by adjusting the Gain values for Red and Blue pixels
  else if (strcmp("JAI Corporation", vendor_name) == 0)
  {
    msg.white_balance_red = 1.0;
    msg.white_balance_green = 1.0;
    msg.white_balance_blue = 1.0;
  }
  // the Basler cameras use the 'BalanceRatioAbs' keyword instead
  else if (strcmp("Basler", vendor_name) == 0)
  {
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Red");
    msg.white_balance_red = aravis::device::feature::get_float(p_device_, "BalanceRatioAbs");
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Green");
    msg.white_balance_green = aravis::device::feature::get_float(p_device_, "BalanceRatioAbs");
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Blue");
    msg.white_balance_blue = aravis::device::feature::get_float(p_device_, "BalanceRatioAbs");
  }
  // the standard way
  else if (implemented_features_["BalanceRatio"] && implemented_features_["BalanceRatioSelector"])
  {
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Red");
    msg.white_balance_red = aravis::device::feature::get_float(p_device_, "BalanceRatio");
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Green");
    msg.white_balance_green = aravis::device::feature::get_float(p_device_, "BalanceRatio");
    aravis::device::feature::set_string(p_device_, "BalanceRatioSelector", "Blue");
    msg.white_balance_blue = aravis::device::feature::get_float(p_device_, "BalanceRatio");
  }

  if (strcmp("Basler", vendor_name) == 0) {
    msg.temperature = static_cast<float>(aravis::device::feature::get_float(p_device_, "TemperatureAbs"));
  }
  else if (implemented_features_["DeviceTemperature"])
  {
    msg.temperature = aravis::device::feature::get_float(p_device_, "DeviceTemperature");
  }

}

void CameraAravisNodelet::controlLostCallback(ArvDevice *p_gv_device, gpointer can_instance)
{
  CameraAravisNodelet *p_can = (CameraAravisNodelet*)can_instance;
  ROS_ERROR("Control to aravis device lost.");
  nodelet::NodeletUnload unload_service;
  unload_service.request.name = p_can->getName();
  if (false == ros::service::call(ros::this_node::getName() + "/unload_nodelet", unload_service))
  {
    ros::shutdown();
  }
}

void CameraAravisNodelet::softwareTriggerLoop()
{
  software_trigger_active_ = true;
  ROS_INFO("Software trigger started.");
  std::chrono::system_clock::time_point next_time = std::chrono::system_clock::now();
  while (ros::ok() && software_trigger_active_)
  {
    next_time += std::chrono::milliseconds(size_t(std::round(1000.0 / config_.softwaretriggerrate)));
    if (std::any_of(cam_pubs_.begin(), cam_pubs_.end(),
      [](image_transport::CameraPublisher pub){ return pub.getNumSubscribers() > 0; })
    )
    {
      aravis::device::execute_command(p_device_, "TriggerSoftware");
    }
    if (next_time > std::chrono::system_clock::now())
    {
      std::this_thread::sleep_until(next_time);
    }
    else
    {
      ROS_WARN("Camera Aravis: Missed a software trigger event.");
      next_time = std::chrono::system_clock::now();
    }
  }
  ROS_INFO("Software trigger stopped.");
}

void CameraAravisNodelet::publishTfLoop(double rate)
{
  // Publish optical transform for the camera
  ROS_WARN("Publishing dynamic camera transforms (/tf) at %g Hz", rate);

  tf_thread_active_ = true;

  ros::Rate loop_rate(rate);

  while (ros::ok() && tf_thread_active_)
  {
    // Update the header for publication
    tf_optical_.header.stamp = ros::Time::now();
    ++tf_optical_.header.seq;
    p_tb_->sendTransform(tf_optical_);

    loop_rate.sleep();
  }
}

void CameraAravisNodelet::discoverFeatures()
{
  implemented_features_.clear();
  if (!p_device_)
    return;

  // get the root node of genicam description
  ArvGc *gc = arv_device_get_genicam(p_device_);
  if (!gc)
    return;

  std::unordered_set<ArvDomNode*> done;
  std::list<ArvDomNode*> todo;
  todo.push_front((ArvDomNode*)arv_gc_get_node(gc, "Root"));

  while (!todo.empty())
  {
    // get next entry
    ArvDomNode *node = todo.front();
    todo.pop_front();

    if (done.find(node) != done.end()) continue;
    done.insert(node);

    const std::string name(arv_dom_node_get_node_name(node));

    // Do the indirection
    if (name[0] == 'p')
    {
      if (name.compare("pInvalidator") == 0)
      {
        continue;
      }
      ArvDomNode *inode = (ArvDomNode*)arv_gc_get_node(gc,
                                                       arv_dom_node_get_node_value(arv_dom_node_get_first_child(node)));
      if (inode)
        todo.push_front(inode);
      continue;
    }

    // check for implemented feature
    if (ARV_IS_GC_FEATURE_NODE(node))
    {
      //if (!(ARV_IS_GC_CATEGORY(node) || ARV_IS_GC_ENUM_ENTRY(node) /*|| ARV_IS_GC_PORT(node)*/)) {
      ArvGcFeatureNode *fnode = ARV_GC_FEATURE_NODE(node);
      const std::string fname(arv_gc_feature_node_get_name(fnode));
      const bool usable = arv_gc_feature_node_is_available(fnode, NULL)
          && arv_gc_feature_node_is_implemented(fnode, NULL);

      ROS_INFO_STREAM_COND(verbose_, "Feature " << fname << " is " << (usable ? "usable" : "not usable"));
      implemented_features_.emplace(fname, usable);
      //}
    }

//		if (ARV_IS_GC_PROPERTY_NODE(node)) {
//			ArvGcPropertyNode* pnode = ARV_GC_PROPERTY_NODE(node);
//			const std::string pname(arv_gc_property_node_get_name(pnode));
//			ROS_INFO_STREAM("Property " << pname << " found");
//		}

    if (ARV_IS_GC_CATEGORY(node)) {
        const GSList* features;
        const GSList* iter;
        features = arv_gc_category_get_features(ARV_GC_CATEGORY(node));
        for (iter = features; iter != NULL; iter = iter->next) {
            ArvDomNode* next = (ArvDomNode*) arv_gc_get_node(gc, (const char*) iter->data);
            todo.push_front(next);
        }

        continue;
    }

    // add children in todo-list
    ArvDomNodeList *children = arv_dom_node_get_child_nodes(node);
    const uint l = arv_dom_node_list_get_length(children);
    for (uint i = 0; i < l; ++i)
    {
      todo.push_front(arv_dom_node_list_get_item(children, i));
    }
  }
}

void CameraAravisNodelet::parseStringArgs(std::string in_arg_string, std::vector<std::string> &out_args) {
  size_t array_start = 0;
  size_t array_end = in_arg_string.length();
  if(array_start != std::string::npos && array_end != std::string::npos) {
        // parse the string into an array of parameters
        std::stringstream ss(in_arg_string.substr(array_start, array_end - array_start));
        while (ss.good()) {
          std::string temp;
          getline( ss, temp, ',');
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
// Read ROS parameters from this node's namespace, and see if each parameter has a similarly named & typed feature in the camera.  Then set the
// camera feature to that value.  For example, if the parameter camnode/Gain is set to 123.0, then we'll write 123.0 to the Gain feature
// in the camera.
//
// Note that the datatype of the parameter *must* match the datatype of the camera feature, and this can be determined by
// looking at the camera's XML file.  Camera enum's are string parameters, camera bools are false/true parameters (not 0/1),
// integers are integers, doubles are doubles, etc.
//
void CameraAravisNodelet::writeCameraFeaturesFromRosparam()
{
  XmlRpc::XmlRpcValue xml_rpc_params;
  XmlRpc::XmlRpcValue::iterator iter;

  getPrivateNodeHandle().getParam(this->getName(), xml_rpc_params);

  if (xml_rpc_params.getType() != XmlRpc::XmlRpcValue::TypeStruct) {  return; }

  // Write first the Selectors given that no meaningful order could be applied (cascade order is lost)
  std::for_each(xml_rpc_params.begin(), xml_rpc_params.end(), [&](auto elem) {
      if (elem.first.find("Selector") != elem.first.npos) this->writeCameraFeatureFromRosparam(elem);
  });

  std::for_each(xml_rpc_params.begin(), xml_rpc_params.end(), [&](auto elem) {
      if (elem.first.find("Selector") == elem.first.npos) this->writeCameraFeatureFromRosparam(elem);
  });
}

void CameraAravisNodelet::writeCameraFeatureFromRosparam(const XmlRpc::XmlRpcValue::iterator::value_type& iter) {
  GError *error = NULL;

  std::string key = iter.first;

  ArvGcNode *p_gc_node = arv_device_get_feature(p_device_, key.c_str());
  if (p_gc_node && arv_gc_feature_node_is_implemented(ARV_GC_FEATURE_NODE(p_gc_node), &error))
  {
    //				unsigned long	typeValue = arv_gc_feature_node_get_value_type((ArvGcFeatureNode *)pGcNode);
    //				ROS_INFO("%s cameratype=%lu, rosparamtype=%d", key.c_str(), typeValue, static_cast<int>(iter.second.getType()));

    // We'd like to check the value types too, but typeValue is often given as G_TYPE_INVALID, so ignore it.
    switch (iter.second.getType())
    {
      case XmlRpc::XmlRpcValue::TypeBoolean: //if ((iter.second.getType()==XmlRpc::XmlRpcValue::TypeBoolean))// && (typeValue==G_TYPE_INT64))
      {
        bool value = (bool)iter.second;
        aravis::device::feature::set_boolean(p_device_, key.c_str(), value);
        ROS_INFO("Read parameter (bool) %s: %s", key.c_str(), value ? "true" : "false");
      }
        break;

      case XmlRpc::XmlRpcValue::TypeInt: //if ((iter.second.getType()==XmlRpc::XmlRpcValue::TypeInt))// && (typeValue==G_TYPE_INT64))
      {
        int value = (int)iter.second;
        aravis::device::feature::set_integer(p_device_, key.c_str(), value);
        ROS_INFO("Read parameter (int) %s: %d", key.c_str(), value);
      }
        break;

      case XmlRpc::XmlRpcValue::TypeDouble: //if ((iter.second.getType()==XmlRpc::XmlRpcValue::TypeDouble))// && (typeValue==G_TYPE_DOUBLE))
      {
        double value = (double)iter.second;
        aravis::device::feature::set_float(p_device_, key.c_str(), value);
        ROS_INFO("Read parameter (float) %s: %f", key.c_str(), value);
      }
        break;

      case XmlRpc::XmlRpcValue::TypeString: //if ((iter.second.getType()==XmlRpc::XmlRpcValue::TypeString))// && (typeValue==G_TYPE_STRING))
      {
        std::string value = (std::string)iter.second;
        aravis::device::feature::set_string(p_device_, key.c_str(), value.c_str());
        ROS_INFO("Read parameter (string) %s: %s", key.c_str(), value.c_str());
      }
        break;

      case XmlRpc::XmlRpcValue::TypeInvalid:
      case XmlRpc::XmlRpcValue::TypeDateTime:
      case XmlRpc::XmlRpcValue::TypeBase64:
      case XmlRpc::XmlRpcValue::TypeArray:
      case XmlRpc::XmlRpcValue::TypeStruct:
      default:
        ROS_WARN("Unhandled rosparam type in writeCameraFeaturesFromRosparam()");
    }
  }
}

} // end namespace camera_aravis
