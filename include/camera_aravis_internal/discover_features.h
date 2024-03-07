#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_DISCOVER_FEATURES_H
#define CAMERA_ARAVIS_INTERNAL_DISCOVER_FEATURES_H


#include <unordered_map>

#include <camera_aravis_internal/aravis_abstraction.h>

namespace camera_aravis::internal {
    std::unordered_map<std::string, const bool> discover_features(ArvDevice* device);
}

#endif
