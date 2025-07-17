#include "CaptureCameraFeed.h"

CaptureCameraFeed::CaptureCameraFeed(int device_index, int capture_width, int capture_height, int framerate)
    : frame_width(capture_width), frame_height(capture_height),frame_count(0), fps_to_display(0.0) // Initialize camera parameters
{
    std::string pipeline = GetGstreamPipeline(device_index,capture_width,capture_height,framerate); // Construct GStreamer pipeline string
    if(pipeline.empty())
    {
        std::cerr << "ERROR: GStreamer pipeline is empty." << std::endl; // Check if the pipeline string is empty
        throw std::runtime_error("GStreamer pipeline is empty");
    }
    
    std::cout << "Using GStreamer pipeline:" << pipeline << std::endl;
    
    cap_.open(pipeline,cv::CAP_GSTREAMER); // Open the camera using the GStreamer pipeline

    if(!cap_.isOpened())
    {
        std::cerr << "ERROR: Failed to opened camera with pipeline." << std::endl; // Check if the camera is opened successfully
        throw std::runtime_error("Failed to open camera");
    }
}

CaptureCameraFeed::~CaptureCameraFeed()
{
    if(cap_.isOpened())
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
    start_time = std::chrono::steady_clock::now();
    while (is_running)
    {
        cv::Mat temp_frame;
        if(!cap_.read(temp_frame))
        {
            std::cerr << ("ERROR: frame could not be captured from the camera") << std::endl;
            break;  
        }

        {
            std::lock_guard<std::mutex> lock(frame_lock); // Lock the frame to ensure thread safety
            if (temp_frame.empty())
            {
                std::cerr << "ERROR: Captured frame is empty." << std::endl;
                continue; // Skip the iteration if the frame is empty
            }
            if (temp_frame.size() != cv::Size(frame_width, frame_height))
            {
                std::cerr << "ERROR: Captured frame size does not match the expected size." << std::endl;
                continue; // Skip the iteration if the frame size does not match
            }
            temp_frame.copyTo(frame); // Copy the captured frame to the class member variable
        }

        cv::flip(frame, frame, 0); // flip the frame

        frame_count++;
        // CalculateFPS(); // Calculate FPS based on the captured frames
        // PrintOnScreen(); // Display the current FPS on the screen

        // cv::imshow("Camera Feed", frame); // Show the captured frame in a window
        if (cv::waitKey(1) == 27)
        {
            break;
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

void CaptureCameraFeed::CalculateFPS()
{
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    if(elapsed_time_ms >= 1000)
    {
        fps_to_display = frame_count * 1000.0 / elapsed_time_ms; // Calculate FPS based on the elapsed time and frame count

        start_time = std::chrono::steady_clock::now();
        frame_count = 0;
    }
}

void CaptureCameraFeed::PrintOnScreen()
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << fps_to_display; 
    std::string fps_text = "FPS: " + ss.str(); // Format the FPS text to display with two decimal places

     cv::putText(frame,
                    fps_text,             
                    cv::Point(10, 30),     
                    cv::FONT_HERSHEY_SIMPLEX, 
                    1.0,                   
                    cv::Scalar(0, 255, 0),  
                    2);  
}