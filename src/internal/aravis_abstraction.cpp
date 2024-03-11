#include <camera_aravis_internal/aravis_abstraction.h>

#include <camera_aravis_internal/GErrorGuard.h>


namespace camera_aravis {
    namespace aravis {
        namespace device {
            void execute_command(const NonOwnedGPtr<ArvDevice>& dev, const char* cmd) {
                GuardedGError err;
                arv_device_execute_command(dev.get(), cmd, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            namespace feature {
                gboolean get_boolean(const NonOwnedGPtr<ArvDevice>& dev, const char* feat) {
                    GuardedGError err;
                    gboolean res = arv_device_get_boolean_feature_value(dev.get(), feat, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                    return res;
                }

                void set_boolean(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gboolean val) {
                    GuardedGError err;
                    arv_device_set_boolean_feature_value(dev.get(), feat, val, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                gint64 get_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat) {
                    GuardedGError err;
                    gint64 res = arv_device_get_integer_feature_value(dev.get(), feat, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                    return res;
                }

                void set_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gint64 val) {
                    GuardedGError err;
                    arv_device_set_integer_feature_value(dev.get(), feat, val, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                double get_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat) {
                    GuardedGError err;
                    double res = arv_device_get_float_feature_value(dev.get(), feat, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                    return res;
                }

                void set_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, double val) {
                    GuardedGError err;
                    arv_device_set_float_feature_value(dev.get(), feat, val, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                const char* get_string(const NonOwnedGPtr<ArvDevice>& dev, const char* feat) {
                    GuardedGError err;
                    const char* res = arv_device_get_string_feature_value(dev.get(), feat, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                    return res;
                }

                void set_string(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, const char* val) {
                    GuardedGError err;
                    arv_device_set_string_feature_value(dev.get(), feat, val, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                namespace bounds {
                    void get_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gint64* min, gint64* max) {
                        GuardedGError err;
                        arv_device_get_integer_feature_bounds(dev.get(), feat, min, max, err.storeError());
                        LOG_GERROR_ARAVIS(err);
                    }

                    void get_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, double* min, double* max) {
                        GuardedGError err;
                        arv_device_get_float_feature_bounds(dev.get(), feat, min, max, err.storeError());
                        LOG_GERROR_ARAVIS(err);
                    }
                }  // namespace bounds
            }      // namespace feature

            bool is_gv(const NonOwnedGPtr<ArvDevice>& dev) { return ARV_IS_GV_DEVICE(dev.get()); }
            bool is_uv(const NonOwnedGPtr<ArvDevice>& dev) { return ARV_IS_UV_DEVICE(dev.get()); }

            gint64 get_num_streams(const NonOwnedGPtr<ArvDevice>& dev) {
                gint64 num_streams = arv_device_get_integer_feature_value(dev.get(), "DeviceStreamChannelCount", nullptr);
                // if this return 0, try the deprecated GevStreamChannelCount in case this is an older camera
                if (!num_streams && is_gv(dev)) {
                    num_streams = arv_device_get_integer_feature_value(dev.get(), "GevStreamChannelCount", nullptr);
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

                void set_usb_mode(const NonOwnedGPtr<ArvDevice>& dev, ArvUvUsbMode usb_mode) {
                    if (is_uv(dev)) {
                        arv_uv_device_set_usb_mode(ARV_UV_DEVICE(dev.get()), usb_mode);
                        ROS_INFO("USB mode: %s", usb_mode_string(usb_mode));
                    }
                }
#endif
            }  // namespace USB3Vision

        }  // namespace device

        GPtr<ArvCamera> camera_new(const char* name) {
            GuardedGError err;
            ArvCamera* res = arv_camera_new(name, err.storeError());
            LOG_GERROR_ARAVIS(err);
            // err.log(logger_suffix);
            return GPtr<ArvCamera>(res);
        }

        namespace camera {

            NonOwnedGPtr<ArvDevice> get_device(const NonOwnedGPtr<ArvCamera>& cam) {
                return NonOwnedGPtr<ArvDevice> (arv_camera_get_device(cam.get()));
            }

            const char* get_vendor_name(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                const char* res = arv_camera_get_vendor_name(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            gint64 get_payload(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                gint64 res = arv_camera_get_payload(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            double get_frame_rate(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                double res = arv_camera_get_frame_rate(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_frame_rate(const NonOwnedGPtr<ArvCamera>& cam, double val) {
                GuardedGError err;
                arv_camera_set_frame_rate(cam.get(), val, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            double get_exposure_time(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                double res = arv_camera_get_exposure_time(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_exposure_time(const NonOwnedGPtr<ArvCamera>& cam, double val) {
                GuardedGError err;
                arv_camera_set_exposure_time(cam.get(), val, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            double get_gain(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                double res = arv_camera_get_gain(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            void set_gain(const NonOwnedGPtr<ArvCamera>& cam, double val) {
                GuardedGError err;
                arv_camera_set_gain(cam.get(), val, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            void get_region(const NonOwnedGPtr<ArvCamera>& cam, gint* x, gint* y, gint* width, gint* height) {
                GuardedGError err;
                arv_camera_get_region(cam.get(), x, y, width, height, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            void set_region(const NonOwnedGPtr<ArvCamera>& cam, gint x, gint y, gint width, gint height) {
                GuardedGError err;
                arv_camera_set_region(cam.get(), x, y, width, height, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            void get_sensor_size(const NonOwnedGPtr<ArvCamera>& cam, gint* width, gint* height) {
                GuardedGError err;
                arv_camera_get_sensor_size(cam.get(), width, height, err.storeError());
                LOG_GERROR_ARAVIS(err);
            }

            bool is_frame_rate_available(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_frame_rate_available(cam.get(), err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            bool is_exposure_time_available(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_exposure_time_available(cam.get(), err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            bool is_gain_available(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                bool res = static_cast<bool>(arv_camera_is_gain_available(cam.get(), err.storeError()));
                LOG_GERROR_ARAVIS(err);
                return res;
            }

            GPtr<ArvStream> create_stream(const NonOwnedGPtr<ArvCamera>& cam, ArvStreamCallback callback, void* user_data) {
                GuardedGError err;
                ArvStream* res = arv_camera_create_stream(cam.get(), callback, user_data, err.storeError());
                LOG_GERROR_ARAVIS(err);
                return GPtr<ArvStream>(res);
            }

            void start_acquisition(const NonOwnedGPtr<ArvCamera>& cam) {
                GuardedGError err;
                arv_camera_start_acquisition(cam.get(), err.storeError());
                LOG_GERROR_ARAVIS(err);
            }


            namespace bounds {

                void get_width(const NonOwnedGPtr<ArvCamera>& cam, gint* min, gint* max) {
                    GuardedGError err;
                    arv_camera_get_width_bounds(cam.get(), min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_height(const NonOwnedGPtr<ArvCamera>& cam, gint* min, gint* max) {
                    GuardedGError err;
                    arv_camera_get_height_bounds(cam.get(), min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_exposure_time(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_exposure_time_bounds(cam.get(), min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_gain(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_gain_bounds(cam.get(), min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }

                void get_frame_rate(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max) {
                    GuardedGError err;
                    arv_camera_get_frame_rate_bounds(cam.get(), min, max, err.storeError());
                    LOG_GERROR_ARAVIS(err);
                }


            }  // namespace bounds

            namespace gv {
                void select_stream_channel(const NonOwnedGPtr<ArvCamera>& cam, gint channel_id) {
                    GuardedGError err;
                    arv_camera_gv_select_stream_channel(cam.get(), channel_id, err.storeError());
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
