#include "LineFollower.h"

std::atomic<bool> LineFollower::shutdown_flag(false);

void LineFollower::signalHandler(int signal)
{
    std::cout << "\nInterrupt signal received, Shutting down" << std::endl;
    shutdown_flag = true;
}

LineFollower::LineFollower(DeltaBot &bot, CaptureCameraFeed &camera)
    : deltabot(bot), cameraFeed(camera), is_running(false)
{
    signal(SIGINT, LineFollower::signalHandler);
    std::cout << "Initializing Neural Network..." << std::endl;
    // Network structure: input layer(7 neurons), hidden layer(10 neurons), output layers(1 neurons)
    int neuronsPerLayer[] = {hidden_neurons, output_neurons};
    // 2 layers(1 hidden layer + 1 output layer)
    neuralNet = std::make_unique<Net>(layer, neuronsPerLayer, input_neurons, output_layer);
    // Initialise network weights and activation functions
    // neuralNet->initNetwork(Neuron::W_RANDOM, Neuron::B_RANDOM, Neuron::Act_Tanh);
    // set learning rate
    neuralNet->setLearningRate(learning_rate);
    std::cout << "Neural network initialisation complete!" << std::endl;
}

void LineFollower::start()
{
    is_running = true;
    shutdown_flag = false;
    std::cout << "Line following and learing start..." << std::endl;

    while (is_running && !shutdown_flag)
    {
        long current_frame_id = cameraFeed.GetFrameId();

        if (current_frame_id > last_process_frame_id)
        {
            last_process_frame_id = current_frame_id;
            cv::Mat frame = cameraFeed.GetFrame();

            if (frame.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            cv::flip(frame, frame, 0);

            std::vector<float> inputs;
            float error_near = 0.0, error_far = 0.0; // Initialize errors
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
                    error_integral = 0.0;
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
                    error_integral = 0.0;
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

            // cv::imshow("Camera Feed", frame);
        }

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

bool LineFollower::ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<float> &inputs, float &error_near, float &error_far)
{
    cv::Mat hsv, binary;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Scalar lower_black = cv::Scalar(0, 0, 0);      // Define the range for black color in HSV space
    cv::Scalar upper_black = cv::Scalar(180, 255, 80); // Define the range for black color in HSV space

    cv::inRange(hsv, lower_black, upper_black, binary);                         // Create a binary image where black pixels are set to 255 and others to 0
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)); // Create a rectangular kernel for morphological operations
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel);                  // Apply closing operation to fill small holes in the binary image

    cv::imshow("Binary Image", binary);

    cv::Rect roi_near_rect(0, frame.rows * 3 / 4, frame.cols, frame.rows / 4); // Near segment at the bottom

    cv::Mat roi_near = binary(roi_near_rect); // Extract the near segment

    cv::Moments m_near = cv::moments(roi_near, true); // Calculate moments for the near segment

    bool line_found_near = false;
    if (m_near.m00 > 100) // Check if the near segment has enough mass
    {
        float cx = m_near.m10 / m_near.m00;
        error_near = (cx - roi_near.cols / 2.0) / (roi_near.cols / 2.0); // Calculate error based on the center of mass
        line_found_near = true;
    }
    else
    {
        error_near = 0.0; // If no line is found in the near segment, set error to 0
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

    return line_found_near;
}

void LineFollower::UpdateAndTrain(const std::vector<float> &inputs, float error_near, float error_far)
{
    neuralNet->setInputs(inputs.data());
    neuralNet->propInputs();
    float nn_raw = neuralNet->getOutput(0);
    float Kp_adapt = kp_min + (nn_raw + 1.0f) * 0.5f * (kp_max - kp_min);

    float p_term = Kp_adapt * error_near; // Proportional term for the controller

    float error_derivative = error_near - last_error;  // Calculate the derivative of the error
    float d_term = error_derivative * derivative_gain; // Derivative term for the controller

    error_integral += error_near;                                                         // Update the integral of the heading error
    float integral_limit = 20.0;                                                          // Limit for the integral term to prevent windup
    error_integral = std::max(-integral_limit, std::min(integral_limit, error_integral)); // Clamp the integral term
    float i_term = error_integral * integral_gain;                                        // Integral term for the controller

    float total_steering_adjustment = p_term + d_term + i_term; // Total steering adjustment based on the controller

    last_error = error_near; // Update the last error for the next iteration

    float left_speed = base_speed - total_steering_adjustment;
    float right_speed = base_speed + total_steering_adjustment;

    left_speed = std::max(-10.0f, std::min(10.0f, left_speed));
    right_speed = std::max(-10.0f, std::min(10.0f, right_speed)); // Clamp the speed values to a range of -10 to 10
    deltabot.SetMotorSpeed(left_speed, right_speed);              // Set the motor speeds based on the calculated adjustments

    ec_lp = ec_alpha * ec_lp + (1.0f - ec_alpha) * error_near; // Low-pass filter for error correction
    if (++train_counter % train_every_n == 0) // Train the model every n frames
    {
        neuralNet->customBackProp(-ec_lp, error_gain); // Perform backpropagation with the low-pass filtered error
        neuralNet->updateWeights();
    }

    std::cout << "Err: " << error_near
              << "| Steer: " << p_term
              << "| Speed L/R: " << left_speed << "/" << right_speed << std::endl;
}