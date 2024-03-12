#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_GERROR_GUARD_H
#define CAMERA_ARAVIS_INTERNAL_GERROR_GUARD_H

#include <memory>

#include <glib.h>

#include <camera_aravis_internal/GErrorROSLog.h>


namespace camera_aravis {

    using GErrorGuard = std::unique_ptr<GError*, void (*)(GError**)>;

    GErrorGuard makeGErrorGuard();

    class GuardedGError {
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
    };

    bool operator==(const GuardedGError& lhs, const GError* rhs);
    bool operator==(const GuardedGError& lhs, const GuardedGError& rhs);
    bool operator!=(const GuardedGError& lhs, std::nullptr_t);

}  // namespace camera_aravis

#endif
