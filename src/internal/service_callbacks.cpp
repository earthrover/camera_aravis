#include <camera_aravis/camera_aravis_nodelet.h>

#include <camera_aravis_internal/GErrorGuard.h>
#include <camera_aravis_internal/GErrorROSLog.h>
#include <camera_aravis_internal/aravis_abstraction.h>

namespace camera_aravis {
    bool CameraAravisNodelet::getIntegerFeatureCallback(camera_aravis::get_integer_feature_value::Request& request,
                                                        camera_aravis::get_integer_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        response.response = arv_device_get_integer_feature_value(this->p_device_, feature_name, error.storeError());
        LOG_GERROR_ARAVIS(error);
        return !error;
    }

    bool CameraAravisNodelet::setIntegerFeatureCallback(camera_aravis::set_integer_feature_value::Request& request,
                                                        camera_aravis::set_integer_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        guint64 value = request.value;
        arv_device_set_integer_feature_value(this->p_device_, feature_name, value, error.storeError());
        LOG_GERROR_ARAVIS(error);
        response.ok = !error;
        return true;
    }

    bool CameraAravisNodelet::getFloatFeatureCallback(camera_aravis::get_float_feature_value::Request& request,
                                                      camera_aravis::get_float_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        response.response = arv_device_get_float_feature_value(this->p_device_, feature_name, error.storeError());
        LOG_GERROR_ARAVIS(error);
        return !error;
    }

    bool CameraAravisNodelet::setFloatFeatureCallback(camera_aravis::set_float_feature_value::Request& request,
                                                      camera_aravis::set_float_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        const double value = request.value;
        arv_device_set_float_feature_value(this->p_device_, feature_name, value, error.storeError());
        LOG_GERROR_ARAVIS(error);
        response.ok = !error;
        return true;
    }

    bool CameraAravisNodelet::getStringFeatureCallback(camera_aravis::get_string_feature_value::Request& request,
                                                       camera_aravis::get_string_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        response.response = arv_device_get_string_feature_value(this->p_device_, feature_name, error.storeError());
        LOG_GERROR_ARAVIS(error);
        return !error;
    }

    bool CameraAravisNodelet::setStringFeatureCallback(camera_aravis::set_string_feature_value::Request& request,
                                                       camera_aravis::set_string_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        const char* value = request.value.c_str();
        arv_device_set_string_feature_value(this->p_device_, feature_name, value, error.storeError());
        LOG_GERROR_ARAVIS(error);
        response.ok = !error;
        return true;
    }

    bool CameraAravisNodelet::getBooleanFeatureCallback(camera_aravis::get_boolean_feature_value::Request& request,
                                                        camera_aravis::get_boolean_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        response.response = arv_device_get_boolean_feature_value(this->p_device_, feature_name, error.storeError());
        LOG_GERROR_ARAVIS(error);
        return !error;
    }

    bool CameraAravisNodelet::setBooleanFeatureCallback(camera_aravis::set_boolean_feature_value::Request& request,
                                                        camera_aravis::set_boolean_feature_value::Response& response) {
        GuardedGError error;
        const char* feature_name = request.feature.c_str();
        const bool value = request.value;
        arv_device_set_boolean_feature_value(this->p_device_, feature_name, value, error.storeError());
        LOG_GERROR_ARAVIS(error);
        response.ok = !error;
        return true;
    }

    bool CameraAravisNodelet::executeCommandCallback(camera_aravis::execute_command::Request& request,
                                                     camera_aravis::execute_command::Response& response) {
        GuardedGError error;
        const char* command = request.command.c_str();
        arv_device_execute_command(this->p_device_, command, error.storeError());
        LOG_GERROR_ARAVIS(error);
        response.response = !error;
        return true;
    }
}  // namespace camera_aravis
