
#include <camera_aravis_internal/tuneGVStream.h>
#include <ros/console.h>

namespace camera_aravis::internal {
    // Extra stream options for GigEVision streams.
    void tuneGvStream(ArvGvStream* p_stream) {
        gboolean b_auto_buffer = FALSE;
        gboolean b_packet_resend = TRUE;
        unsigned int timeout_packet = 40;  // milliseconds
        unsigned int timeout_frame_retention = 200;

        if (p_stream) {
            if (!ARV_IS_GV_STREAM(p_stream)) {
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
            g_object_set(p_stream, "packet-timeout", timeout_packet * 1000, "frame-retention",
                         timeout_frame_retention * 1000, NULL);
        }
    }
}
