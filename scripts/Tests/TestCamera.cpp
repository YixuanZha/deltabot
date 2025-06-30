#include "CaptureCameraFeed.h"

int main(int argc, char* argv[])
{
    const int DEVICE_INDEX = 11;    
    const int CAPTURE_WIDTH = 1680;
    const int CAPTURE_HEIGHT = 1050;
    const int CAPTURE_FPS = 30;

    try
    {
        CaptureCameraFeed camera(DEVICE_INDEX,CAPTURE_WIDTH,CAPTURE_HEIGHT,CAPTURE_FPS);
        camera.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "An expection occurred: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}