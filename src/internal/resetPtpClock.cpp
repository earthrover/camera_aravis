#include <camera_aravis_internal/resetPtpClock.h>
#include <camera_aravis_internal/aravis_abstraction.h>
#include <ros/console.h>

namespace camera_aravis::internal {
    void resetPtpClock(ArvDevice* dev) {
        // a PTP slave can take the following states: Slave, Listening, Uncalibrated, Faulty, Disabled
        const std::string ptp_status = aravis::device::feature::get_string(dev, "GevIEEE1588Status");

        if (ptp_status == std::string("Faulty") || ptp_status == std::string("Disabled")) {
            ROS_INFO("camera_aravis: Reset ptp clock (was set to %s)", ptp_status.c_str());

            aravis::device::feature::set_boolean(dev, "GevIEEE1588", false);
            aravis::device::feature::set_boolean(dev, "GevIEEE1588", true);
        }
    }
}  // namespace camera_aravis::internal
