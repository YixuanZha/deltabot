#ifndef DELTABOT_H
#define DELTABOT_H

#include "ServoMotorSetting.h"
#include "CaptureCameraFeed.h"
#include <thread>

class DeltaBot
{
public:
    DeltaBot();

    /**
     * Move the robot forward.
     * @param  {float} speed : The speed to move forward.
     */
    void Forward(float speed);

    /**
     * Move the robot backward.
     * @param  {float} speed : The speed to move backward.
     */
    void Backward(float speed);

    /**
     * Move the robot to the left.
     * @param  {float} speed : The speed to move to the left.
     */
    void TurnLeft(float speed);

    /**
     * Move the robot to the right.
     * @param  {float} speed : The speed to move to the right.
     */
    void TurnRight(float speed);

    void SetMotorSpeed(float left_speed,float right_speed);

    /**
     * Start the camera feed in a separate thread.
     * This allows the robot to capture video feed while performing other operations.
     */
    void StartCamera();
    
    /**
     * Stop the camera feed and join the camera thread.
     * This ensures that the camera resources are released properly.
     */
    void StopCamera();

    /**
     * Stop the robot.
     */
    void Stop();

private:
    // Servo motor settings for left and right motors
    ServoMotorSetting leftMotor;
    ServoMotorSetting rightMotor;

    // Camera feed capture
    CaptureCameraFeed camera;
    std::thread camera_thread;

    // Constants for servo motor settings
    static const int leftChannel = 0;
    static const int rightChannel = 0;
    static constexpr float low_time = 20.0f;
    static constexpr float high_time = 1.52f;
    static const int leftChipNo = 8;
    static const int rightChipNo = 14;
    
    // Camera parameters
    static const int device_index = 11;
    static const int capture_width = 1280;
    static const int capture_height = 720;
    static const int capture_fps = 30;
};

#endif