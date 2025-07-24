#ifndef CAMERA_FPS_TRACKER_H
#define CAMERA_FPS_TRACKER_H

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

/**
 * @brief Class to track frames per second (FPS) of the camera feed.
 * It calculates the FPS based on the number of frames captured in a second.
 * The FPS value can be retrieved as a formatted string.
 */
class CameraFPSTracker
{
public:
    CameraFPSTracker();
    
    /**
     * @brief Call this function to update the FPS tracker with the latest frame.
     */
    void tick();

    /**
     * @brief Get the current FPS value as a formatted string.
     * @return {std::string}  : The formatted FPS string.
     */
    std::string getFPSText() const;

private:
    int frame_count;
    double current_fps;
    std::chrono::time_point<std::chrono::steady_clock> start_time;
};

#endif