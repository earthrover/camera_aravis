#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_GERROR_ROS_LOG_H
#define CAMERA_ARAVIS_INTERNAL_GERROR_ROS_LOG_H

#include <string>

#include <glib.h>

#include <ros/console.h>

#define _LOG_GERROR_ARAVIS_IMPL(err, suffix)                                                                          \
    ROS_ERROR_COND_NAMED((err) != nullptr, suffix, "[%s] Code %i: %s", g_quark_to_string((err)->domain), (err)->code, \
                         (err)->message)

#ifdef ARAVIS_ERRORS_ABORT
#define LOG_GERROR_ARAVIS(err)                                                                               \
    ROS_ASSERT_MSG((err) == nullptr, "%s: [%s] Code %i: %s", ::camera_aravis::aravis::logger_suffix.c_str(), \
                   g_quark_to_string((err)->domain), (err)->code, (err)->message)
#else
#define LOG_GERROR_ARAVIS(err) _LOG_GERROR_ARAVIS_IMPL(err, ::camera_aravis::aravis::logger_suffix)
#endif


namespace camera_aravis {
    namespace aravis {
        const std::string logger_suffix = "aravis";
    }
}  // namespace camera_aravis

#endif
