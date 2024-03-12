#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_RESET_PTP_CLOCK_H
#define CAMERA_ARAVIS_INTERNAL_RESET_PTP_CLOCK_H

#include <camera_aravis_internal/GPtr.h>

extern "C" {
#include <arv.h>
}

namespace camera_aravis::internal {
    // reset PTP clock
    void resetPtpClock(const NonOwnedGPtr<ArvDevice>& dev);
}  // namespace camera_aravis::internal

#endif
