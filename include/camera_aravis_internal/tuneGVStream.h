#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_TUNE_GV_STREAM_H
#define CAMERA_ARAVIS_INTERNAL_TUNE_GV_STREAM_H


extern "C" {
#include <arv.h>
}

namespace camera_aravis::internal {
    // Extra stream options for GigEVision streams.
    void tuneGvStream(ArvGvStream* p_stream);
}  // namespace camera_aravis::internal

#endif
