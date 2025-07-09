#include "DeltaBot.h"
#include "CaptureCameraFeed.h"
#include "LineFollower.h"
#include <iostream>
#include <thread>

int main()
{
    DeltaBot deltabot;
    CaptureCameraFeed camera(10,640,480,40);

    std::thread camera_thread(&CaptureCameraFeed::run, &camera);
    std::cout << "Camera thread has started" << std::endl;

    LineFollower lineFollower(deltabot,camera);

    lineFollower.start();

    std::cout << "Stopping camera thread" << std::endl;
    camera.stop();

    if(camera_thread.joinable())
    {
        camera_thread.join();
    }

    return 0;
}