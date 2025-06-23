#include "CaptureCameraFeed.h"

int main(int argc, char* argv[])
{
    const int CAMERA_ID = 11;
    CaptureCameraFeed app(CAMERA_ID);
    app.run();

    return 0;
}