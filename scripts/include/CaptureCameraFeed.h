#ifndef CAPTURECAMERAFEED_H
#define CAPTURECAMERAFEED_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

class CaptureCameraFeed
{
public:
    CaptureCameraFeed(int camera_id);
    void run();

private:
    int camera_id_;
    cv::VideoCapture cap_;

    cv::Mat preprocess(const cv::Mat &frame);
    
    void postprocess(const std::vector<float> &output_tensor, cv::Mat &frame);
};

#endif