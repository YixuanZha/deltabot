#include "DeltaBot.h"

DeltaBot::DeltaBot()
    : leftMotor(leftChannel, low_time, high_time, leftChipNo), rightMotor(rightChannel, low_time, high_time, rightChipNo)
{
}

void DeltaBot::Forward(float speed)
{
    leftMotor.ChangeLeftSpeed(speed);
    rightMotor.ChangeRightSpeed(-speed);
}

void DeltaBot::Backward(float speed)
{
    leftMotor.ChangeLeftSpeed(-speed);
    rightMotor.ChangeRightSpeed(speed);
}

void DeltaBot::TurnLeft(float speed)
{
    leftMotor.ChangeLeftSpeed(speed / 2.0f);
    rightMotor.ChangeRightSpeed(-speed);
}

void DeltaBot::TurnRight(float speed)
{
    leftMotor.ChangeLeftSpeed(speed);
    rightMotor.ChangeRightSpeed(-speed / 2.0f);
}

void DeltaBot::Stop()
{
    leftMotor.StopMotor();
    rightMotor.StopMotor();
}