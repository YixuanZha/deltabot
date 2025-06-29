#ifndef CAPTURECAMERAFEED_H
#define CAPTURECAMERAFEED_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>

class CaptureCameraFeed
{
public:
    CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate);
    void run();

private:
    int frame_width;
    int frame_height;
    int frame_count = 0;
    double fps_to_display;
    cv::Mat frame;

    std::chrono::time_point<std::chrono::steady_clock> start_time;

    cv::VideoCapture cap_;

    std::string GetGstreamPipeline(int device_index, int width, int height, int fps);

    void CalculateFPS();

    void PrintOnScreen();

    cv::Mat preprocess(const cv::Mat &frame);
    
    void postprocess(const std::vector<float> &output_tensor, cv::Mat &frame);
};

#endif