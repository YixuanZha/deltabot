#ifndef CAPTURECAMERAFEED_H
#define CAPTURECAMERAFEED_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <atomic>

/**
 * CaptureCameraFeed class captures video feed from a camera using OpenCV and GStreamer.
 * It processes the frames to calculate and display the frames per second (FPS) on the screen.
 */
class CaptureCameraFeed
{
public:
    /**
     * Constructor for CaptureCameraFeed.
     * @param device_index : The index of the camera device to capture from.
     * @param capture_width : The width of the captured frames.
     * @param capture_height : The height of the captured frames.
     * @param framerate : The desired frame rate for capturing video.
     * @note The GStreamer pipeline is constructed based on the provided parameters.
     */
    CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate);
    
    /**
     * Destructor for CaptureCameraFeed.
     * It releases the camera resource if it is opened.
     */
    ~CaptureCameraFeed();

    /**
     * Starts capturing video feed from the camera in a separate thread.
     * It continuously reads frames, calculates FPS, and displays the frames on the screen.
     */
    void run();

    /**
     * Stops the camera feed capture.
     * It sets the running flag to false, which will stop the capturing loop.
     */
    void stop();

private:
    // Camera parameters
    int frame_width; 
    int frame_height;
    int frame_count;
    double fps_to_display;

    // Frame to capture video feed
    cv::Mat frame;

    std::chrono::time_point<std::chrono::steady_clock> start_time;

    cv::VideoCapture cap_;

    std::atomic<bool> is_running; // Flag to control the running state of the camera feed capture

    std::string GetGstreamPipeline(int device_index, int width, int height, int fps);

    /**
     * Calculates the frames per second (FPS) of the captured video feed.
     */
    void CalculateFPS();

    /**
     * Displays the current FPS on the screen.  
     */
    void PrintOnScreen();
};

#endif