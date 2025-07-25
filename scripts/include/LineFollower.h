#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include "DeltaBot.h"
#include "CaptureCameraFeed.h"
#include "Net.h"
#include "CameraFPSTracker.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>
#include <memory>
#include <algorithm>
#include <iomanip>

// Enum to represent the state of the robot
enum RobotState
{
    FOLLOWING, // Following the line
    SEARCHING_TURN // Searching for the line by turning
};
/**
 * @brief Class for line following behavior
 */
class LineFollower
{
public:
    /**
     * @brief Constructor for LineFollower
     * @param bot : Reference to the DeltaBot instance for controlling the robot
     * @param camera : Reference to the CaptureCameraFeed instance for capturing video feed
     */
    LineFollower(DeltaBot &bot, CaptureCameraFeed &camera);

    /**
     * @brief Starts the line following and learning process
     */
    void start();

    /**
     * @brief Stops the line following and learning process
     */
    void stop();

private:
    /**
     * @brief Processes the captured frame to extract inputs for the neural network
     * @param frame : The captured frame from the camera
     * @param inputs : Vector to store the processed inputs for the neural network
     * @param error_near : Reference to store the error calculated from the near segment
     * @param error_far : Reference to store the error calculated from the far segment
     * @return true if the line is found, false otherwise
     * @note The function processes the frame to create a binary image, extracts regions of interest
     * for near and far segments, calculates the moments to find the center of the line,
     * and computes the errors based on the center positions.
     */
    bool ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<double> &inputs, double &error_near, double &error_far);

    /**
     * @brief Updates the neural network with the current inputs and trains it
     * @param inputs : The processed inputs for the neural network
     * @param error_near : The error calculated from the near segment
     * @param error_far : The error calculated from the far segment
     * @note The function sets the inputs to the neural network, propagates the inputs,
     * and performs backpropagation to update the weights based on the errors.
     */
    void UpdateAndTrain(const std::vector<double> &inputs, double error_near, double error_far);

    // Reference to the DeltaBot and CaptureCameraFeed instances
    DeltaBot &deltabot;
    CaptureCameraFeed &cameraFeed;
    CameraFPSTracker fpsTracker;

    RobotState currentState = FOLLOWING; // Initial state is following the line
    double last_known_error = 0.0; // Last known error for line following

    std::atomic<bool> is_running; // Flag to control the running state of the line follower

    std::unique_ptr<Net> neuralNet; // Neural network for line following

    // Constants for line following
    const static constexpr int layer = 2;
    const static constexpr int input_neurons = 7;
    const static constexpr int hidden_neurons = 10;
    const static constexpr int output_neurons = 1;
    const static constexpr int output_layer = 1;

    // figure configuration
    float base_speed = 1.0f;
    int binary_threshold = 100;

    // learning rate and error gain
    double learning_rate = 0.01f;
    double error_gain = 5;

    // controller parameters
    double last_error = 0.0; 
    float proportional_gain = 0.8f; // Proportional gain for the controller
    float derivative_gain = 0.3f; // Derivative gain for the controller
    float heading_gain = 0.6f; // Heading gain for the controller
};

#endif