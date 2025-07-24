#include "LineFollower.h"

LineFollower::LineFollower(DeltaBot &bot, CaptureCameraFeed &camera)
    : deltabot(bot), cameraFeed(camera), is_running(false)
{
    std::cout << "Initializing Neural Network..." << std::endl;
    // Network structure: input layer(7 neurons), hidden layer(10 neurons), output layers(2 neurons)
    int neuronsPerLayer[] = {10, 2};
    // 2 layers(1 hidden layer + 1 output layer)
    neuralNet = std::make_unique<Net>(2, neuronsPerLayer, input_segments, 1);
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

        std::vector<double> inputs;
        double training_error = ProcessFrameAndGetInputs(frame, inputs);

        if (!inputs.empty() && training_error != 0.0)
        {
            UpdateAndTrain(inputs, training_error);
        }
        else
        {
            deltabot.Stop();
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

double LineFollower::ProcessFrameAndGetInputs(const cv::Mat &frame, std::vector<double> &inputs)
{
    cv::Mat gray, binary;
    // Converting colour image to greyscale
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // Processes only the lower third of the image
    cv::Rect roi(0, frame.rows * 2 / 3, frame.cols, frame.rows / 3);
    cv::Mat roi_gray = gray(roi);
    // Turns a greyscale image into a binary image with only pure black and white.
    cv::threshold(roi_gray, binary, binary_thresdhold, 255, cv::THRESH_BINARY_INV);

    inputs.clear();
    // The binarised image was cut horizontally into seven vertical bands
    int segment_width = binary.cols / input_segments;
    for (int i = 0; i < input_segments; ++i)
    {
        // Calculate the white pixel density for each region and normalise to [0, 1]
        cv::Rect segment_roi(i * segment_width, 0, segment_width, binary.rows);
        cv::Mat segment = binary(segment_roi);
        double density = cv::mean(segment)[0] / 255.0;
        inputs.push_back(density);
    }

    cv::Moments m = cv::moments(binary, true);
    double training_error = 0.0;
    if (m.m00 > 0) // m00 -> sum of white pixel points
    {
        double cx = m.m10 / m.m00; // cx -> x-coordinate of the centre of mass , m10 -> the cumulative sum of the x-coordinates of the white pixels
        double center = binary.cols / 2.0;
        training_error = (cx - center) / center;
    }

    cv::imshow("Binary ROI", binary);
    return training_error;
}

void LineFollower::UpdateAndTrain(const std::vector<double> &inputs, double training_error)
{
    neuralNet->setInputs(inputs.data());
    neuralNet->propInputs();

    double left_output = neuralNet->getOutput(0);
    double right_output = neuralNet->getOutput(1);

    float left_adjustment = (float)(left_output * steering_gain);
    float right_adjustment = (float)(right_output * steering_gain);

    float left_speed = base_speed + left_adjustment;
    float right_speed = base_speed + right_adjustment;

    left_speed = std::max(-10.0f, std::min(10.0f, left_speed));
    right_speed = std::max(-10.0f, std::min(10.0f, right_speed));

    deltabot.SetMotorSpeed(left_speed, right_speed);

    std::cout << "Error: " << std::fixed << std::setprecision(2) << training_error
              << "| Net Out L/R: " << left_output << "/" << right_output
              << "| Speed L/R: " << left_speed << "/" << right_speed << std::endl;

    if (training_error != 0.0)
    {
        int lastLayerIndex = neuralNet->getnLayers() - 1;
        Layer *outputLayer = neuralNet->getLayer(lastLayerIndex);

        double amplified_error = training_error * error_gain;

        outputLayer->getNeuron(0)->setInternalError(0, amplified_error, Neuron::Value);
        outputLayer->getNeuron(1)->setInternalError(0, -amplified_error, Neuron::Value);

        std::vector<int> injection_layers = {lastLayerIndex, 0};

        neuralNet->customBackProp(injection_layers, 0, 0, Neuron::Value, false);

        neuralNet->updateWeights();
    }
}