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

#ifndef CAMERA_ARAVIS_CAMERA_ARAVIS_NODELET
#define CAMERA_ARAVIS_CAMERA_ARAVIS_NODELET

extern "C" {
#include <arv.h>
}

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <functional>
#include <cctype>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <unordered_map>

#include <glib.h>

#include <ros/ros.h>
#include <nodelet/nodelet.h>
#include <nodelet/NodeletUnload.h>
#include <ros/time.h>
#include <ros/duration.h>
#include <sensor_msgs/Image.h>
#include <std_msgs/Int64.h>
#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>
#include <boost/algorithm/string/trim.hpp>

#include <dynamic_reconfigure/server.h>
#include <dynamic_reconfigure/SensorLevels.h>
#include <tf/transform_listener.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_broadcaster.h>
#include <camera_aravis/CameraAravisConfig.h>
#include <camera_aravis/CameraAutoInfo.h>
#include <camera_aravis/ExtendedCameraInfo.h>

#include <camera_aravis/get_integer_feature_value.h>
#include <camera_aravis/set_integer_feature_value.h>
#include <camera_aravis/get_float_feature_value.h>
#include <camera_aravis/set_float_feature_value.h>
#include <camera_aravis/get_string_feature_value.h>
#include <camera_aravis/set_string_feature_value.h>
#include <camera_aravis/get_boolean_feature_value.h>
#include <camera_aravis/set_boolean_feature_value.h>

#include <camera_aravis/execute_command.h>

#include <camera_aravis/camera_buffer_pool.h>
#include <camera_aravis/conversion_utils.h>

namespace camera_aravis {

    typedef CameraAravisConfig Config;

    class CameraAravisNodelet : public nodelet::Nodelet {
        public:
        CameraAravisNodelet() {};
        virtual ~CameraAravisNodelet();

        private:
        std::string guid_ = "";
        std::string frame_id_ = "";
        bool use_ptp_stamp_ = false;

        ArvCamera* p_camera_ = NULL;
        ArvDevice* p_device_ = NULL;

        gint num_streams_ = 0;
        std::vector<std::string> stream_names_;
        int32_t acquire_ = 0;

        virtual void onInit() override;
        void spawnStream();


        protected:
        struct Sensor {
            int32_t width = 0;
            int32_t height = 0;
            std::string pixel_format;
            size_t n_bits_pixel = 0;
        };

        struct Stream {
            std::string name;
            ArvStream* arv_stream;
            Sensor sensor_description;
            CameraBufferPool::Ptr buffer_pool;
            std::unique_ptr<camera_info_manager::CameraInfoManager> camera_info_manager;
            ros::NodeHandle camera_info_node_handle;
            sensor_msgs::CameraInfoPtr camera_info;
            image_transport::CameraPublisher camera_publisher;
            ConversionFunction conversion_function;
        };

        void print_capabilities();

        // Start and stop camera on demand
        void rosConnectCallback();

        // Callback to wrap and send recorded image as ROS message
        static void newBufferReady(Stream& stream,
                                   std::string frame_id,
                                   int32_t width,
                                   int32_t height,
                                   bool use_ptp_stamp);

        // Clean-up if aravis device is lost
        static void controlLostCallback(ArvDevice* p_gv_device, gpointer can_instance);

        // Services
        ros::ServiceServer get_integer_service_;
        bool getIntegerFeatureCallback(camera_aravis::get_integer_feature_value::Request& request,
                                       camera_aravis::get_integer_feature_value::Response& response);

        ros::ServiceServer get_float_service_;
        bool getFloatFeatureCallback(camera_aravis::get_float_feature_value::Request& request,
                                     camera_aravis::get_float_feature_value::Response& response);

        ros::ServiceServer get_string_service_;
        bool getStringFeatureCallback(camera_aravis::get_string_feature_value::Request& request,
                                      camera_aravis::get_string_feature_value::Response& response);

        ros::ServiceServer get_boolean_service_;
        bool getBooleanFeatureCallback(camera_aravis::get_boolean_feature_value::Request& request,
                                       camera_aravis::get_boolean_feature_value::Response& response);

        ros::ServiceServer set_integer_service_;
        bool setIntegerFeatureCallback(camera_aravis::set_integer_feature_value::Request& request,
                                       camera_aravis::set_integer_feature_value::Response& response);

        ros::ServiceServer set_float_service_;
        bool setFloatFeatureCallback(camera_aravis::set_float_feature_value::Request& request,
                                     camera_aravis::set_float_feature_value::Response& response);

        ros::ServiceServer set_string_service_;
        bool setStringFeatureCallback(camera_aravis::set_string_feature_value::Request& request,
                                      camera_aravis::set_string_feature_value::Response& response);

        ros::ServiceServer set_boolean_service_;
        bool setBooleanFeatureCallback(camera_aravis::set_boolean_feature_value::Request& request,
                                       camera_aravis::set_boolean_feature_value::Response& response);

        ros::ServiceServer exec_command_service_;
        bool executeCommandCallback(camera_aravis::execute_command::Request& request,
                                    camera_aravis::execute_command::Response& response);

        void shutdown();

        static void parseStringArgs(const std::string& in_arg_string, std::vector<std::string>& out_args);

        // WriteCameraFeaturesFromRosparam()
        // Read ROS parameters from this node's namespace, and see if each parameter has a similarly named & typed
        // feature in the camera.  Then set the camera feature to that value.  For example, if the parameter
        // camnode/Gain is set to 123.0, then we'll write 123.0 to the Gain feature in the camera.
        //
        // Note that the datatype of the parameter *must* match the datatype of the camera feature, and this can be
        // determined by looking at the camera's XML file.  Camera enum's are string parameters, camera bools are
        // false/true parameters (not 0/1), integers are integers, doubles are doubles, etc.
        void writeCameraFeaturesFromRosparam();
        void writeCameraFeatureFromRosparam(const XmlRpc::XmlRpcValue::iterator::value_type& iter);


        std::atomic<bool> spawning_;
        std::thread spawn_stream_thread_;

        ros::Timer software_trigger_timer_;

        std::unordered_map<std::string, const bool> implemented_features_;

        struct {
            int32_t x = 0;
            int32_t y = 0;
            int32_t width = 0;
            int32_t width_min = 0;
            int32_t width_max = 0;
            int32_t height = 0;
            int32_t height_min = 0;
            int32_t height_max = 0;
        } roi_;


        struct StreamIdData {
            CameraAravisNodelet* can;
            size_t stream_id;
        };

        std::vector<StreamIdData> stream_ids_;

        std::vector<Stream> streams_;
    };

}  // end namespace camera_aravis

#endif
