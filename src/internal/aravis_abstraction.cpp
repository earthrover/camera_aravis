#include <camera_aravis_internal/aravis_abstraction.h>

namespace camera_aravis {
    namespace aravis {
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
                }  // namespace bounds
            }      // namespace feature

            bool is_gv(ArvDevice* dev) { return ARV_IS_GV_DEVICE(dev); }
            bool is_uv(ArvDevice* dev) { return ARV_IS_UV_DEVICE(dev); }

            gint64 get_num_streams(ArvDevice* dev) {
                gint64 num_streams = arv_device_get_integer_feature_value(dev, "DeviceStreamChannelCount", nullptr);
                // if this return 0, try the deprecated GevStreamChannelCount in case this is an older camera
                if (!num_streams && ARV_IS_GV_DEVICE(dev)) {
                    num_streams = arv_device_get_integer_feature_value(dev, "GevStreamChannelCount", nullptr);
                }

                return num_streams;
            }

            namespace USB3Vision {
#if ARAVIS_HAS_USB_MODE
                const char* usb_mode_string(ArvUvUsbMode usb_mode) {
                    switch (usb_mode) {
                        case ARV_UV_USB_MODE_SYNC: return "ARV_UV_USB_MODE_SYNC";
                        case ARV_UV_USB_MODE_ASYNC: return "ARV_UV_USB_MODE_ASYNC";
                        // case ARV_UV_USB_MODE_DEFAULT: return "ARV_UV_USB_MODE_DEFAULT";
                        default: return "Unhandled_aravis_usb_mode";
                    }
                }

                void set_usb_mode(ArvDevice* dev, ArvUvUsbMode usb_mode) {
                    if (is_uv(dev)) {
                        arv_uv_device_set_usb_mode(ARV_UV_DEVICE(dev), usb_mode);
                        ROS_INFO("USB mode: %s", usb_mode_string(usb_mode));
                    }
                }
#endif
            }  // namespace USB3Vision

        }  // namespace device

        ArvCamera* camera_new(const char* name) {
            GuardedGError err;
            ArvCamera* res = arv_camera_new(name, err.storeError());
            LOG_GERROR_ARAVIS(err);
            // err.log(logger_suffix);
            return res;
        }

        namespace camera {

            const char* get_vendor_name(ArvCamera* cam) {
                GuardedGError err;
                const char* res = arv_camera_get_vendor_name(cam, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            gint64 get_payload(ArvCamera* cam) {
                GuardedGError err;
                gint64 res = arv_camera_get_payload(cam, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            double get_frame_rate(ArvCamera* cam) {
                GuardedGError err;
                double res = arv_camera_get_frame_rate(cam, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_frame_rate(ArvCamera* cam, double val) {
                GuardedGError err;
                arv_camera_set_frame_rate(cam, val, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            double get_exposure_time(ArvCamera* cam) {
                GuardedGError err;
                double res = arv_camera_get_exposure_time(cam, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_exposure_time(ArvCamera* cam, double val) {
                GuardedGError err;
                arv_camera_set_exposure_time(cam, val, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            double get_gain(ArvCamera* cam) {
                GuardedGError err;
                double res = arv_camera_get_gain(cam, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_gain(ArvCamera* cam, double val) {
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

            bool is_frame_rate_available(ArvCamera* cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_frame_rate_available(cam, err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            bool is_exposure_time_available(ArvCamera* cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_exposure_time_available(cam, err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            bool is_gain_available(ArvCamera* cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_gain_available(cam, err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
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

                void get_width(ArvCamera* cam, gint* min, gint* max) {
                    GuardedGError err;
                    arv_camera_get_width_bounds(cam, min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_height(ArvCamera* cam, gint* min, gint* max) {
                    GuardedGError err;
                    arv_camera_get_height_bounds(cam, min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_exposure_time(ArvCamera* cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_exposure_time_bounds(cam, min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_gain(ArvCamera* cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_gain_bounds(cam, min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_frame_rate(ArvCamera* cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_frame_rate_bounds(cam, min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }


            }  // namespace bounds

            namespace gv {
                void select_stream_channel(ArvCamera* cam, gint channel_id) {
                    GuardedGError err;
                    arv_camera_gv_select_stream_channel(cam, channel_id, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }
            }  // namespace gv
        }      // namespace camera

        namespace buffer{
            const char* status_string(ArvBufferStatus status) {
                switch (status) {
                    case ARV_BUFFER_STATUS_UNKNOWN: return "ARV_BUFFER_STATUS_UNKNOWN ";
                    case ARV_BUFFER_STATUS_SUCCESS: return "ARV_BUFFER_STATUS_SUCCESS";
                    case ARV_BUFFER_STATUS_CLEARED: return "ARV_BUFFER_STATUS_CLEARED";
                    case ARV_BUFFER_STATUS_TIMEOUT: return "ARV_BUFFER_STATUS_TIMEOUT";
                    case ARV_BUFFER_STATUS_MISSING_PACKETS: return "ARV_BUFFER_STATUS_MISSING_PACKETS";
                    case ARV_BUFFER_STATUS_WRONG_PACKET_ID: return "ARV_BUFFER_STATUS_WRONG_PACKET_ID";
                    case ARV_BUFFER_STATUS_SIZE_MISMATCH: return "ARV_BUFFER_STATUS_SIZE_MISMATCH";
                    case ARV_BUFFER_STATUS_FILLING: return "ARV_BUFFER_STATUS_FILLING";
                    case ARV_BUFFER_STATUS_ABORTED: return "ARV_BUFFER_STATUS_ABORTED";
                    // case ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED: return "ARV_BUFFER_STATUS_PAYLOAD_NOT_SUPPORTED";
                    default: return "Unhandled_aravis_buffer_status";
                }
            }
        }
    }          // namespace aravis
}  // namespace camera_aravis
