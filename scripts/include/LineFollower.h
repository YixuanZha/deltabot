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

enum RobotState
{
    FOLLOWING,
    SEARCHING_TURN
};

class LineFollower
{
public:
    LineFollower(DeltaBot &bot, CaptureCameraFeed &camera);

    void start();

    void stop();

private:
    bool ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<double> &inputs,double& training_error);

    void UpdateAndTrain(const std::vector<double> &inputs, double training_error);

    DeltaBot &deltabot;
    CaptureCameraFeed &cameraFeed;

    RobotState current_state = FOLLOWING;
    double last_known_error = 0.0;

    std::atomic<bool> is_running;

    std::unique_ptr<Net> neuralNet;

    const int input_segments = 7;
    float base_speed = 1.0f;
    int binary_thresdhold = 100;
    double learning_rate = 0.01f;
    double error_gain = 5;

    double last_error = 0.0;
    float kp = 0.8f;
    float kd = 0.4f;
};

#endif