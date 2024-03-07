#include <camera_aravis_internal/GErrorGuard.h>


namespace camera_aravis {

    GErrorGuard makeGErrorGuard() {
        return GErrorGuard(nullptr, [](GErrorGuard::pointer error) {
            if (error && *error) g_error_free(*error);
        });
    }

    /*class GuardedGError {
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

        void log(const std::string& suffix = "") { _LOG_GERROR_ARAVIS_IMPL(err, suffix); }

        friend bool operator==(const GuardedGError& lhs, const GError* rhs);
        friend bool operator==(const GuardedGError& lhs, const GuardedGError& rhs);
        friend bool operator!=(const GuardedGError& lhs, std::nullptr_t);

        private:
        GError* err = nullptr;
    };*/

    bool operator==(const GuardedGError& lhs, const GError* rhs) { return lhs.err == rhs; }
    bool operator==(const GuardedGError& lhs, const GuardedGError& rhs) { return lhs.err == rhs.err; }
    bool operator!=(const GuardedGError& lhs, std::nullptr_t) { return !!lhs; }

}  // namespace camera_aravis
