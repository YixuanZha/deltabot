#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include "DeltaBot.h"
#include "CaptureCameraFeed.h"
#include "Net.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <algorithm>
#include <iomanip>

class LineFollower
{
public:
    LineFollower(DeltaBot &bot, CaptureCameraFeed &camera);

    void start();

    void stop();

private:
    double ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<double> &inputs);

    void UpdateAndTrain(const std::vector<double> &inputs, double training_error);

    DeltaBot &deltabot;
    CaptureCameraFeed &cameraFeed;
    std::atomic<bool> is_running;

    std::unique_ptr<Net> neuralNet;

    const int input_segments = 7;
    float speed = 1.0f;
    int binary_threadhold = 100;
    double learning_rate = 0.005f;
};

#endif