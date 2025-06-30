#include "CaptureCameraFeed.h"

CaptureCameraFeed::CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate)
    : frame_width(capture_width), frame_height(capture_height),frame_count(0), fps_to_display(0.0)
{
    std::string pipeline = GetGstreamPipeline(device_index,capture_width,capture_height,framerate);
    std::cout << "Using GStreamer pipeline:" << pipeline << std::endl;

    cap_.open(pipeline,cv::CAP_GSTREAMER);

    if(!cap_.isOpened())
    {
        std::cerr << "ERROR: Failed to opened camera with pipeline." << std::endl;
        throw std::runtime_error("Failed to open camera");
    }
}

void CaptureCameraFeed::run()
{
    start_time = std::chrono::steady_clock::now();
    while (true)
    {
        if(!cap_.read(frame))
        {
            std::cerr << ("ERROR: frame could not be captured from the camera") << std::endl;
            break;  
        }

        cv::flip(frame, frame, 0); // flip the frame

        frame_count++;
        CalculateFPS();
        PrintOnScreen();

        cv::imshow("Camera Feed", frame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }
}

std::string CaptureCameraFeed::GetGstreamPipeline(int device_index, int width, int height, int fps)
{
     return "v4l2src device=/dev/video" + std::to_string(device_index) +
           " io-mode=4 ! video/x-raw, width=" + std::to_string(width) +
           ", height=" + std::to_string(height) + ", framerate=" +
           std::to_string(fps) + "/1 ! videoconvert ! " +
           "video/x-raw,format=NV12 ! appsink";
}

void CaptureCameraFeed::CalculateFPS()
{
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if(elapsed_time_ms >= 1000)
    {
        fps_to_display = frame_count * 1000.0 / elapsed_time_ms;

        start_time = std::chrono::steady_clock::now();
        frame_count = 0;
    }
}

void CaptureCameraFeed::PrintOnScreen()
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << fps_to_display;
    std::string fps_text = "FPS: " + ss.str();

     cv::putText(frame,
                    fps_text,             
                    cv::Point(10, 30),     
                    cv::FONT_HERSHEY_SIMPLEX, 
                    1.0,                   
                    cv::Scalar(0, 255, 0),  
                    2);  
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