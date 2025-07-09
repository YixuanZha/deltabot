#include "DeltaBot.h"

DeltaBot::DeltaBot()
    : leftMotor(leftChannel, low_time, high_time, leftChipNo),
      rightMotor(rightChannel, low_time, high_time, rightChipNo),
      camera(device_index, capture_width, capture_height, capture_fps) // Initialize the left and right motors and the camera feed capture
{
}

void DeltaBot::Forward(float speed) // Move the robot forward
{
    leftMotor.ChangeLeftSpeed(speed);
    rightMotor.ChangeRightSpeed(-speed);
}

void DeltaBot::Backward(float speed) // Move the robot backward
{
    leftMotor.ChangeLeftSpeed(-speed);
    rightMotor.ChangeRightSpeed(speed);
}

void DeltaBot::TurnLeft(float speed) // Turn the robot to the left
{
    leftMotor.ChangeLeftSpeed(speed / 2.0f);
    rightMotor.ChangeRightSpeed(-speed);
}

void DeltaBot::TurnRight(float speed) // Turn the robot to the right
{
    leftMotor.ChangeLeftSpeed(speed);
    rightMotor.ChangeRightSpeed(-speed / 2.0f);
}

void DeltaBot::Stop() // Stop the robot
{
    leftMotor.StopMotor();
    rightMotor.StopMotor();
}

void DeltaBot::SetMotorSpeed(float left_speed, float right_speed)
{
    leftMotor.ChangeLeftSpeed(left_speed);
    rightMotor.ChangeRightSpeed(-right_speed);
}

void DeltaBot::StartCamera() // Start the camera feed
{
    if(camera_thread.joinable())
    {
        std::cout << "Camera thread is already running" << std::endl;
        return;
    }
    camera_thread = std::thread(&CaptureCameraFeed::run,&camera);
    std::cout << "Camera thread started" << std::endl;
}

void DeltaBot::StopCamera() // Stop the camera feed
{
    camera.stop();
    if(camera_thread.joinable())
    {
        camera_thread.join();
        std::cout << "Camera thread joined" << std::endl;
    }
}