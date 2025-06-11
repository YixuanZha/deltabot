#include "DeltaBot.h"

DeltaBot::DeltaBot()
    : leftMotor(leftChannel, low_time, high_time, leftChipNo), rightMotor(rightChannel, low_time, high_time, rightChipNo)
{
}

void DeltaBot::Forward(float speed)
{
    leftMotor.ChangeSpeed(speed);
    rightMotor.ChangeSpeed(-speed);
}

void DeltaBot::Backward(float speed)
{
    leftMotor.ChangeSpeed(-speed);
    rightMotor.ChangeSpeed(speed);
}

void DeltaBot::TurnLeft(float speed)
{
    leftMotor.ChangeSpeed(speed / 2.0f);
    rightMotor.ChangeSpeed(-speed);
}

void DeltaBot::TurnRight(float speed)
{
    leftMotor.ChangeSpeed(speed);
    rightMotor.ChangeSpeed(-speed / 2.0f);
}

void DeltaBot::Stop()
{
    leftMotor.StopMotor();
    rightMotor.StopMotor();
}