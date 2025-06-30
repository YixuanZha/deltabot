#ifndef CAPTURECAMERAFEED_H
#define CAPTURECAMERAFEED_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>

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
     * @throws std::runtime_error if the camera cannot be opened.
     * @note The GStreamer pipeline is constructed based on the provided parameters.
     * @note The camera feed is flipped vertically to match the expected orientation.
     */
    CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate);
    
    /**
     * Destructor for CaptureCameraFeed.
     * It releases the camera resource if it is opened.
     */
    ~CaptureCameraFeed();
    void run();

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

    std::string GetGstreamPipeline(int device_index, int width, int height, int fps);

    /**
     * Calculates the frames per second (FPS) of the captured video feed.
     */
    void CalculateFPS();

    /**
     * Displays the current FPS on the screen.  
     */
    void PrintOnScreen();

    // cv::Mat preprocess(const cv::Mat &frame);
    
    // void postprocess(const std::vector<float> &output_tensor, cv::Mat &frame);
};

#endif