#include "LineFollower.h"

LineFollower::LineFollower(DeltaBot &bot, CaptureCameraFeed &camera)
    : deltabot(bot), cameraFeed(camera), is_running(false)
{
    std::cout << "Initializing Neural Network..." << std::endl;
    // Network structure: input layer(7 neurons), hidden layer(10 neurons), output layers(1 neurons)
    int neuronsPerLayer[] = {hidden_neurons, output_neurons};
    // 2 layers(1 hidden layer + 1 output layer)
    neuralNet = std::make_unique<Net>(layer, neuronsPerLayer, input_neurons, output_layer);
    // Initialise network weights and activation functions
    neuralNet->initNetwork(Neuron::W_RANDOM, Neuron::B_RANDOM, Neuron::Act_Tanh);
    // set learning rate
    neuralNet->setLearningRate(learning_rate);
    std::cout << "Neural network initialisation complete!" << std::endl;
}

void LineFollower::start()
{
    is_running = true;
    std::cout << "Line following and learing start..." << std::endl;

    while (is_running)
    {
        cv::Mat frame = cameraFeed.GetFrame();
        if (frame.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        cv::flip(frame, frame, 0);

        std::vector<double> inputs;
        double error_near = 0.0, error_far = 0.0; // Initialize errors
        bool line_found = ProcessFrameAndGetInputs(frame, inputs, error_near, error_far);

        switch (currentState)
        {
        case FOLLOWING: // If the line is found, update and train the neural network
            if (line_found)
            {
                UpdateAndTrain(inputs, error_near, error_far);
                if (std::abs(error_near) > 0.1)
                {
                    last_known_error = error_near;
                }
            }
            else
            {
                currentState = SEARCHING_TURN;
                std::cout << "Line lost!" << std::endl;
                heading_error_integral = 0.0;
                deltabot.Stop();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            break;
        case SEARCHING_TURN: // If the line is not found, turn the robot to search for the line
            if (line_found)
            {
                currentState = FOLLOWING;
                std::cout << "Line re-found!" << std::endl;
                last_error = 0.0;
                heading_error_integral = 0.0;
            }
            else
            {
                float turn_speed = 3.0f;
                if (last_known_error > 0)
                {
                    deltabot.SetMotorSpeed(turn_speed, -turn_speed); // Turn right
                }
                else
                {
                    deltabot.SetMotorSpeed(-turn_speed, turn_speed); // Turn left
                }
            }
            break;
        }

        fpsTracker.tick();
        std::string fps_text = fpsTracker.getFPSText();
        cv::putText(frame, fps_text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow("Camera Feed", frame);
        if (cv::waitKey(1) == 27)
        {
            stop();
        }
    }
    stop();
}

void LineFollower::stop()
{
    is_running = false;
    deltabot.Stop();
    cv::destroyAllWindows();
}

// +-----------------------------------------------------------------------------------+
// |                                Binarised ROI area                                 |
// +-----------+-----------+-----------+-----------+-----------+-----------+-----------+
// | Segment 1 | Segment 2 | Segment 3 | Segment 4 | Segment 5 | Segment 6 | Segment 7 |
// |           |           |        * *|* *        |           |           |           |
// |           |           |        * *|* *        |           |           |           |
// |           |           |        * *|* *        |           |           |           |
// |           |           |        * *|* *        |           |           |           |
// |           |           |        * *|* *        |           |           |           |
// +-----------+-----------+-----------+-----------+-----------+-----------+-----------+
// |density:0.0|    0.0    |    0.2    |    0.2    |    0.0    |    0.0    |    0.0    |
// +-----------+-----------+-----------+-----------+-----------+-----------+-----------+

bool LineFollower::ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<double> &inputs, double &error_near, double &error_far)
{
    cv::Mat gray, binary;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, binary_threshold, 255, cv::THRESH_BINARY_INV);

    cv::Rect roi_near_rect(0, frame.rows * 3 / 4, frame.cols, frame.rows / 4); // Near segment at the bottom
    cv::Rect roi_far_rect(0, frame.rows * 2 / 4, frame.cols, frame.rows / 4);  // Far segment in the middle

    cv::Mat roi_near = binary(roi_near_rect); // Extract the near segment
    cv::Mat roi_far = binary(roi_far_rect);   // Extract the far segment

    cv::Moments m_near = cv::moments(roi_near, true); // Calculate moments for the near segment
    cv::Moments m_far = cv::moments(roi_far, true);   // Calculate moments for the far segment

    bool line_found_near = false;
    if (m_near.m00 > 100) // Check if the near segment has enough mass
    {
        double cx = m_near.m10 / m_near.m00;
        error_near = (cx - roi_near.cols / 2.0) / (roi_near.cols / 2.0); // Calculate error based on the center of mass
        line_found_near = true;
    }
    else
    {
        error_near = 0.0; // If no line is found in the near segment, set error to 0
    }

    if (m_far.m00 > 100) // Check if the far segment has enough mass
    {
        double cx = m_far.m10 / m_far.m00;
        error_far = (cx - roi_far.cols / 2.0) / (roi_far.cols / 2.0); // Calculate error based on the center of mass
    }
    else
    {
        error_far = error_near; // If no line is found in the far segment, use the near segment error
    }

    inputs.clear();
    int segment_width = roi_near.cols / input_neurons;
    for (int i = 0; i < input_neurons; ++i)
    {
        cv::Rect segment_roi(i * segment_width, 0, segment_width, roi_near.rows);
        cv::Mat segment = roi_near(segment_roi);
        inputs.push_back(cv::mean(segment)[0] / 255.0);
    }

    cv::rectangle(frame, roi_near_rect, cv::Scalar(0, 255, 0), 2);
    cv::rectangle(frame, roi_far_rect, cv::Scalar(0, 0, 255), 2);

    return line_found_near;
}

void LineFollower::UpdateAndTrain(const std::vector<double> &inputs, double error_near, double error_far)
{
    neuralNet->setInputs(inputs.data());
    neuralNet->propInputs();
    double nn_steering_output = neuralNet->getOutput(0); // Get the neural network output for steering

    float p_term = nn_steering_output * proportional_gain; // Proportional term for the controller

    double error_derivative = error_near - last_error; // Calculate the derivative of the error
    float d_term = error_derivative * derivative_gain; // Derivative term for the controller

    double heading_error = error_far - error_near; // Calculate the heading error based on the near and far segment errors
    float h_term = heading_error * heading_gain;   // Heading term for the controller

    heading_error_integral += heading_error;
    double integral_limit = 20.0;
    heading_error_integral = std::max(-integral_limit,std::min(integral_limit,heading_error_integral));
    float i_term = heading_error_integral * integral_gain;

    float total_steering_adjustment = p_term + h_term + d_term + i_term; // Total steering adjustment based on the controller

    last_error = error_near; // Update the last error for the next iteration

    float left_speed = base_speed - total_steering_adjustment;
    float right_speed = base_speed + total_steering_adjustment;

    left_speed = std::max(-10.0f, std::min(10.0f, left_speed));
    right_speed = std::max(-10.0f, std::min(10.0f, right_speed)); // Clamp the speed values to a range of -10 to 10
    deltabot.SetMotorSpeed(left_speed, right_speed);              // Set the motor speeds based on the calculated adjustments

    double network_error = error_near - nn_steering_output;
    double amplified_error = network_error * error_gain;

    int lastLayerIndex = neuralNet->getnLayers() - 1;
    std::vector<int> injection_layers = {lastLayerIndex, 0};
    neuralNet->customBackProp(injection_layers, 0, amplified_error, Neuron::Value, false);
    neuralNet->updateWeights();

    std::cout << "Err(Near/Far): " << std::fixed << std::setprecision(2) << error_near << "/" << error_far
              << "| HeadingErr: " << heading_error
              << "| Steer(P/H): " << p_term << "/" << h_term
              << "| Speed L/R: " << left_speed << "/" << right_speed << std::endl;
}