#include "ServoMotorSetting.h"

ServoMotorSetting::ServoMotorSetting(int channel, float low_time, float high_time, int chipNo)
{
    pwm.StartPWM(channel, low_time, high_time, chipNo);
}

ServoMotorSetting::~ServoMotorSetting()
{
    pwm.disable();
}

// high time range from 1.3ms to 1.7ms: 1.3 is full speed clockwise, 1.52 is no rotation, 1.7 is full speed counterclockwise
int ServoMotorSetting::SpeedToHighTimeLeft(float speed) // speed range from -10 to 10, 0 is stop
{
    if (speed > 10)
        speed = 10;
    if (speed < -10)
        speed = -10;
    // left 1523500 right 1521500
    int neutral_time = 1524000; // When speed is 0, The high time is 1520000
    int step_size = 1000;

    return neutral_time + (speed * step_size);
}

int ServoMotorSetting::SpeedToHighTimeRight(float speed) // speed range from -10 to 10, 0 is stop
{
    if (speed > 10)
        speed = 10;
    if (speed < -10)
        speed = -10;
    // left 1523500 right 1521500
    int neutral_time = 1521000; // When speed is 0, The high time is 1520000
    int step_size = 1000;

    return neutral_time + (speed * step_size);
}


void ServoMotorSetting::ChangeLeftSpeed(float speed)
{
    int high_time = SpeedToHighTimeLeft(speed);
    int per = INVERSEDUTY + high_time;
    pwm.SetPeriod(per);
    pwm.SetDutyCycleNS(INVERSEDUTY);
}

void ServoMotorSetting::ChangeRightSpeed(float speed)
{
    int high_time = SpeedToHighTimeRight(speed);
    int per = INVERSEDUTY + high_time;
    pwm.SetPeriod(per);
    pwm.SetDutyCycleNS(INVERSEDUTY);
}

void ServoMotorSetting::StopMotor()
{
    ChangeLeftSpeed(0);
    ChangeRightSpeed(0);
}
