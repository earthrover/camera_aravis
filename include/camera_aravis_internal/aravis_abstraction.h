#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_ARAVIS_ABSTRACTION_H
#define CAMERA_ARAVIS_INTERNAL_ARAVIS_ABSTRACTION_H

#include <camera_aravis_internal/GErrorGuard.h>

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
            void execute_command(ArvDevice* dev, const char* cmd);

            namespace feature {
                gboolean get_boolean(ArvDevice* dev, const char* feat);

                void set_boolean(ArvDevice* dev, const char* feat, gboolean val);

                gint64 get_integer(ArvDevice* dev, const char* feat);

                void set_integer(ArvDevice* dev, const char* feat, gint64 val);

                double get_float(ArvDevice* dev, const char* feat);

                void set_float(ArvDevice* dev, const char* feat, double val);

                const char* get_string(ArvDevice* dev, const char* feat);

                void set_string(ArvDevice* dev, const char* feat, const char* val);

                namespace bounds {
                    void get_integer(ArvDevice* dev, const char* feat, gint64* min, gint64* max);

                    void get_float(ArvDevice* dev, const char* feat, double* min, double* max);
                }  // namespace bounds
            }      // namespace feature

            bool is_gv(ArvDevice* dev);
            bool is_uv(ArvDevice* dev);

            gint64 get_num_streams(ArvDevice* dev);

            namespace USB3Vision {
#if ARAVIS_HAS_USB_MODE
                void set_usb_mode(ArvDevice* dev, ArvUvUsbMode usb_mode);
#endif
            }  // namespace USB3Vision

        }  // namespace device

        ArvCamera* camera_new(const char* name = NULL);

        namespace camera {

            const char* get_vendor_name(ArvCamera* cam);

            gint64 get_payload(ArvCamera* cam);

            double get_frame_rate(ArvCamera* cam);

            void set_frame_rate(ArvCamera* cam, double val);

            double get_exposure_time(ArvCamera* cam);

            void set_exposure_time(ArvCamera* cam, double val);

            double get_gain(ArvCamera* cam);

            void set_gain(ArvCamera* cam, double val);

            void get_region(ArvCamera* cam, gint* x, gint* y, gint* width, gint* height);

            void set_region(ArvCamera* cam, gint x, gint y, gint width, gint height);

            void get_sensor_size(ArvCamera* cam, gint* width, gint* height);

            ArvStream* create_stream(ArvCamera* cam, ArvStreamCallback callback, void* user_data);

            void start_acquisition(ArvCamera* cam);


            namespace bounds {

                void get_width(ArvCamera* cam, gint* min, gint* max);

                void get_height(ArvCamera* cam, gint* min, gint* max);

                void get_exposure_time(ArvCamera* cam, double* min, double* max);

                void get_gain(ArvCamera* cam, double* min, double* max);

                void get_frame_rate(ArvCamera* cam, double* min, double* max);

            }  // namespace bounds

            namespace gv {
                void select_stream_channel(ArvCamera* cam, gint channel_id);
            }  // namespace gv
        }      // namespace camera
    }          // namespace aravis
}  // namespace camera_aravis

#endif
