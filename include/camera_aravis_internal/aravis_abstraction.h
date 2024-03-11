#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_ARAVIS_ABSTRACTION_H
#define CAMERA_ARAVIS_INTERNAL_ARAVIS_ABSTRACTION_H

#include <camera_aravis_internal/GPtr.h>

extern "C" {
#include <arv.h>
}

#ifndef ARAVIS_HAS_USB_MODE
#if ARAVIS_CHECK_VERSION(0, 8, 17)
#define ARAVIS_HAS_USB_MODE 1
#else
#define ARAVIS_HAS_USB_MODE 0
#endif
#endif


namespace camera_aravis {
    namespace aravis {
        namespace device {
            void execute_command(const NonOwnedGPtr<ArvDevice>& dev, const char* cmd);

            namespace feature {
                gboolean get_boolean(const NonOwnedGPtr<ArvDevice>& dev, const char* feat);

                void set_boolean(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gboolean val);

                gint64 get_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat);

                void set_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gint64 val);

                double get_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat);

                void set_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, double val);

                const char* get_string(const NonOwnedGPtr<ArvDevice>& dev, const char* feat);

                void set_string(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, const char* val);

                namespace bounds {
                    void get_integer(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, gint64* min, gint64* max);

                    void get_float(const NonOwnedGPtr<ArvDevice>& dev, const char* feat, double* min, double* max);
                }  // namespace bounds
            }      // namespace feature

            bool is_gv(const NonOwnedGPtr<ArvDevice>& dev);
            bool is_uv(const NonOwnedGPtr<ArvDevice>& dev);

            gint64 get_num_streams(const NonOwnedGPtr<ArvDevice>& dev);

            namespace USB3Vision {
#if ARAVIS_HAS_USB_MODE
                void set_usb_mode(const NonOwnedGPtr<ArvDevice>& dev, ArvUvUsbMode usb_mode);
#endif
            }  // namespace USB3Vision

        }  // namespace device

        GPtr<ArvCamera> camera_new(const char* name = NULL);

        namespace camera {

            NonOwnedGPtr<ArvDevice> get_device(const NonOwnedGPtr<ArvCamera>& cam);

            const char* get_vendor_name(const NonOwnedGPtr<ArvCamera>& cam);

            gint64 get_payload(const NonOwnedGPtr<ArvCamera>& cam);

            double get_frame_rate(const NonOwnedGPtr<ArvCamera>& cam);

            void set_frame_rate(const NonOwnedGPtr<ArvCamera>& cam, double val);

            double get_exposure_time(const NonOwnedGPtr<ArvCamera>& cam);

            void set_exposure_time(const NonOwnedGPtr<ArvCamera>& cam, double val);

            double get_gain(const NonOwnedGPtr<ArvCamera>& cam);

            void set_gain(const NonOwnedGPtr<ArvCamera>& cam, double val);

            void get_region(const NonOwnedGPtr<ArvCamera>& cam, gint* x, gint* y, gint* width, gint* height);

            void set_region(const NonOwnedGPtr<ArvCamera>& cam, gint x, gint y, gint width, gint height);

            void get_sensor_size(const NonOwnedGPtr<ArvCamera>& cam, gint* width, gint* height);

            bool is_frame_rate_available(const NonOwnedGPtr<ArvCamera>& cam);

            bool is_exposure_time_available(const NonOwnedGPtr<ArvCamera>& cam);

            bool is_gain_available(const NonOwnedGPtr<ArvCamera>& cam);

            GPtr<ArvStream> create_stream(const NonOwnedGPtr<ArvCamera>& cam, ArvStreamCallback callback, void* user_data);

            void start_acquisition(const NonOwnedGPtr<ArvCamera>& cam);


            namespace bounds {

                void get_width(const NonOwnedGPtr<ArvCamera>& cam, gint* min, gint* max);

                void get_height(const NonOwnedGPtr<ArvCamera>& cam, gint* min, gint* max);

                void get_exposure_time(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max);

                void get_gain(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max);

                void get_frame_rate(const NonOwnedGPtr<ArvCamera>& cam, double* min, double* max);

            }  // namespace bounds

            namespace gv {
                void select_stream_channel(const NonOwnedGPtr<ArvCamera>& cam, gint channel_id);
            }  // namespace gv
        }      // namespace camera

        namespace buffer {
            // Conversions from integers to Arv types.
            const char* status_string(ArvBufferStatus status);

        }
    }          // namespace aravis
}  // namespace camera_aravis

#endif
