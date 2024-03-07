#include <camera_aravis_internal/GErrorGuard.h>


namespace camera_aravis {

    GErrorGuard makeGErrorGuard() {
        return GErrorGuard(nullptr, [](GErrorGuard::pointer error) {
            if (error && *error) g_error_free(*error);
        });
    }

    bool operator==(const GuardedGError& lhs, const GError* rhs) { return lhs.err == rhs; }
    bool operator==(const GuardedGError& lhs, const GuardedGError& rhs) { return lhs.err == rhs.err; }
    bool operator!=(const GuardedGError& lhs, std::nullptr_t) { return !!lhs; }

}  // namespace camera_aravis
