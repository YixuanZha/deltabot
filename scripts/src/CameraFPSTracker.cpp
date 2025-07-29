#include "CameraFPSTracker.h"

CameraFPSTracker::CameraFPSTracker() : frame_count(0), current_fps(0.0), start_time(std::chrono::steady_clock::now())
{
}

void CameraFPSTracker::tick() // Update the frame count and calculate FPS
{
    frame_count++;
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if (elapsed_time_ms >= 1000)
    {
        current_fps = frame_count * 1000.0 / elapsed_time_ms;
        start_time = std::chrono::steady_clock::now();
        frame_count = 0;
    }
}

std::string CameraFPSTracker::getFPSText() const // Get the current FPS value as a formatted string
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << current_fps;
    return "FPS: " + ss.str();
}