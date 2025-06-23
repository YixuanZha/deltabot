#include "CaptureCameraFeed.h"

CaptureCameraFeed::CaptureCameraFeed(int camera_id)
    : camera_id_(camera_id)
{
    cap_.open(camera_id_);
    if (!cap_.isOpened())
    {
        std::cerr << "ERROR: Can not open camera:" << camera_id_ << std::endl;
        throw std::runtime_error("Failed to open the camera");
    }
}

void CaptureCameraFeed::run()
{
    cv::Mat frame;
    while (true)
    {
        cap_ >> frame;
        if (frame.empty())
        {
            std::cerr << "ERROR: Null framed captured" << std::endl;
            break;
        }

        cv::flip(frame,frame,0); // flip the frame
        cv::Mat DisplayFrame;
        int DisplayWidth = 800;
        int DisplayHight = static_cast<int>(static_cast<double>(frame.rows * DisplayWidth) / frame.cols);
        cv::resize(frame, DisplayFrame, cv::Size(DisplayWidth, DisplayHight));

        cv::imshow("Camera Feed", DisplayFrame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }
}

cv::Mat CaptureCameraFeed::preprocess(const cv::Mat &frame)
{
    cv::Mat resized_frame;
    cv::resize(frame, resized_frame, cv::Size(224, 224));
    return resized_frame;
}

void CaptureCameraFeed::postprocess(const std::vector<float> &output_tensor, cv::Mat &frame)
{
    cv::putText(frame, "Processing results...", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}