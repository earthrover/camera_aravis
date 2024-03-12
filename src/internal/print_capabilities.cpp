
#include <camera_aravis/camera_aravis_nodelet.h>
#include <camera_aravis_internal/aravis_abstraction.h>


#define btoa(x) ((x) ? "true" : "false")
#define btoA(x) ((x) ? "True" : "False")

namespace camera_aravis {
    void CameraAravisNodelet::print_capabilities() {
        ROS_INFO("    Using Camera Configuration:");
        ROS_INFO("    ---------------------------");
        ROS_INFO("    Vendor name          = %s", aravis::device::feature::get_string(device, "DeviceVendorName"));
        ROS_INFO("    Model name           = %s", aravis::device::feature::get_string(device, "DeviceModelName"));
        ROS_INFO("    Device id            = %s", aravis::device::feature::get_string(device, "DeviceUserID"));
        ROS_INFO("    Serial number        = %s", aravis::device::feature::get_string(device, "DeviceSerialNumber"));
        const char* device_type = aravis::device::is_uv(device)
                                      ? "USB3Vision"
                                      : (aravis::device::is_gv(device) ? "GigEVision" : "Other");
        ROS_INFO("    Type                 = %s", device_type);
        ROS_INFO("    Sensor width         = %d", streams_[0].sensor_description.width);
        ROS_INFO("    Sensor height        = %d", streams_[0].sensor_description.height);
        ROS_INFO("    ROI x,y,w,h          = %d, %d, %d, %d", roi_.x, roi_.y, roi_.width, roi_.height);
        ROS_INFO("    Pixel format         = %s", streams_[0].sensor_description.pixel_format.c_str());
        ROS_INFO("    BitsPerPixel         = %lu", streams_[0].sensor_description.n_bits_pixel);

        const char* acquisition_mode = implemented_features_["AcquisitionMode"]
                                           ? aravis::device::feature::get_string(device, "AcquisitionMode")
                                           : "(not implemented in camera)";
        ROS_INFO("    Acquisition Mode     = %s", acquisition_mode);

        const char* trigger_mode = implemented_features_["TriggerMode"]
                                       ? aravis::device::feature::get_string(device, "TriggerMode")
                                       : "(not implemented in camera)";
        ROS_INFO("    Trigger Mode         = %s", trigger_mode);

        const char* trigger_source = implemented_features_["TriggerSource"]
                                         ? aravis::device::feature::get_string(device, "TriggerSource")
                                         : "(not implemented in camera)";
        ROS_INFO("    Trigger Source       = %s", trigger_source);

        bool frame_rate_available = aravis::camera::is_frame_rate_available(camera);
        ROS_INFO("    Can set FrameRate:     %s", btoA(frame_rate_available));
        if (implemented_features_["AcquisitionFrameRate"]) {
            ROS_INFO("    AcquisitionFrameRate = %g hz", aravis::camera::get_frame_rate(camera));
        }


        bool exposure_available = aravis::camera::is_exposure_time_available(camera);
        ROS_INFO("    Can set Exposure:      %s", btoA(exposure_available));
        if (exposure_available) {
            ROS_INFO("    Can set ExposureAuto:  %s", btoA(implemented_features_["ExposureAuto"]));

            double min, max, val = aravis::camera::get_exposure_time(camera);
            aravis::camera::bounds::get_exposure_time(camera, &min, &max);
            ROS_INFO("    Exposure             = %g us in range [%g,%g]", val, min, max);
        }


        bool gain_available = aravis::camera::is_gain_available(camera);
        ROS_INFO("    Can set Gain:          %s", btoA(gain_available));
        if (gain_available) {
            ROS_INFO("    Can set GainAuto:      %s", btoA(implemented_features_["GainAuto"]));

            double min, max, val = aravis::camera::get_gain(camera);
            aravis::camera::bounds::get_gain(camera, &min, &max);
            ROS_INFO("    Gain                 = %f %% in range [%f,%f]", val, min, max);
        }


        ROS_INFO("    Can set FocusPos:      %s", btoA(implemented_features_["FocusPos"]));

        if (implemented_features_["GevSCPSPacketSize"]) {
            ROS_INFO("    Network mtu          = %lu",
                     aravis::device::feature::get_integer(device, "GevSCPSPacketSize"));
        }

        ROS_INFO("    ---------------------------");
    }
}  // namespace camera_aravis
