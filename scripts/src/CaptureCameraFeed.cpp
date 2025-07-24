#include "CaptureCameraFeed.h"

CaptureCameraFeed::CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate)
    : frame_width(capture_width), frame_height(capture_height), frame_count(0) // Initialize camera parameters
{
    std::string pipeline = GetGstreamPipeline(device_index, capture_width, capture_height, framerate); // Construct GStreamer pipeline string
    if (pipeline.empty())
    {
        std::cerr << "ERROR: GStreamer pipeline is empty." << std::endl; // Check if the pipeline string is empty
        throw std::runtime_error("GStreamer pipeline is empty");
    }

    std::cout << "Using GStreamer pipeline:" << pipeline << std::endl;

    cap_.open(pipeline, cv::CAP_GSTREAMER); // Open the camera using the GStreamer pipeline

    if (!cap_.isOpened())
    {
        std::cerr << "ERROR: Failed to opened camera with pipeline." << std::endl; // Check if the camera is opened successfully
        throw std::runtime_error("Failed to open camera");
    }
}

CaptureCameraFeed::~CaptureCameraFeed()
{
    if (cap_.isOpened())
    {
        std::cout << "Release camera resourse ...." << std::endl;
        cap_.release();
    }
}

cv::Mat CaptureCameraFeed::GetFrame()
{
    std::lock_guard<std::mutex> lock(frame_lock); // Lock the frame to ensure thread safety
    return frame.clone();
}

void CaptureCameraFeed::run()
{
    is_running = true;
    while (is_running)
    {
        cv::Mat temp_frame;
        if (!cap_.read(temp_frame))
        {
            std::cerr << ("frame could not be captures from the camera") << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(frame_lock);
            frame = temp_frame;
        }
    }
}

void CaptureCameraFeed::stop()
{
    is_running = false;
}

std::string CaptureCameraFeed::GetGstreamPipeline(int device_index, int width, int height, int fps)
{
    return "v4l2src device=/dev/video" + std::to_string(device_index) +
           " io-mode=4 ! video/x-raw, width=" + std::to_string(width) +
           ", height=" + std::to_string(height) + ", framerate=" +
           std::to_string(fps) + "/1 ! videoconvert ! " +
           "video/x-raw,format=BGR ! appsink"; // Construct GStreamer pipeline string
}