#ifndef SERVO_MOTOR_SETTING_H
#define SERVO_MOTOR_SETTING_H
#include <iostream>
#include "Pwm.h"

#define INVERSEDUTY 20000000

class ServoMotorSetting
{
public:
    /**
     * ServoMotorSetting 
     * 
     * @param  {int} channel     : The GPIO pin number for the motor
     * @param  {float} low_time  : The low time duration for the PWM signal
     * @param  {float} high_time : The high time duration for the PWM signal
     * @param  {int} chipNo      : The chip number for the motor
     */
    ServoMotorSetting(int channel, float low_time, float high_time, int chipNo);

    ~ServoMotorSetting();

    /**
     * Change the speed of the motor.
     * @param  {float} speed : The speed to set for the motor.
     */
    void ChangeSpeed(float speed);
    
    /**
     * Stop the motor.
     */
    void StopMotor();

private:
    PWM pwm; // Assuming PWM is a class that handles PWM operations
    /**
     * Convert speed to high time for PWM signal.
     * @param  {float} speed : The speed to convert.
     * @return {int}         : The high time duration for the PWM signal.
     */
    int SpeedToHighTime(float speed);
};

#endif