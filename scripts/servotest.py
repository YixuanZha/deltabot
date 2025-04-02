#!/usr/bin/env python3
# -- encoding: utf-8 --

from periphery import PWM
import time

# Parallax Servo-specific PWM settings
SERVO_FREQUENCY = 50  # 50Hz (20ms period)
SERVO_STOP_DUTY = 0  # 7.5% (1.5ms pulse) - Stop

SERVO_CW_MAX_DUTY_R = 1- 0.06  # 6.5% (1.3ms pulse) - Max Counterclockwise
SERVO_CCW_MAX_DUTY_R = 1- 0.08  # 8.5% (1.7ms pulse) - Max Clockwise

SERVO_CW_MAX_DUTY_L = 1- 0.08  # 8.5% (1.7ms pulse) - Max Clockwise
SERVO_CCW_MAX_DUTY_L = 1- 0.06  # 6.5% (1.3ms pulse) - Max Counterclockwise

# Get PWM chip and channel from user
pwmchip_R= 14
channel_R = 0

pwmchip_L= 8
channel_L = 0

# Initialize PWM
try:
    PWM_R = PWM(pwmchip_R, channel_R)
    PWM_R.frequency = SERVO_FREQUENCY  # Set frequency to 50Hz
    PWM_R.enable()

    PWM_L = PWM(pwmchip_L, channel_L)
    PWM_L.frequency = SERVO_FREQUENCY  # Set frequency to 50Hz
    PWM_L.enable()

    print("\nControlling Parallax Continuous Rotation Servo...\nPress CTRL+C to stop.")

    while True:
        # Stop (Neutral position)
        PWM_R.duty_cycle = SERVO_STOP_DUTY
        PWM_L.duty_cycle = SERVO_STOP_DUTY
        print("Servo: Stopped (Neutral)")
        time.sleep(2)

        # Rotate Clockwise
        PWM_R.duty_cycle = SERVO_CW_MAX_DUTY_R
        PWM_L.duty_cycle = SERVO_CW_MAX_DUTY_L        
        print("Servo: Rotating Clockwise")
        time.sleep(5)

        # Stop Again
        PWM_R.duty_cycle = SERVO_STOP_DUTY
        PWM_L.duty_cycle = SERVO_STOP_DUTY
        print("Servo: Stopped (Neutral)")
        time.sleep(2)

        # Rotate Counterclockwise
        PWM_R.duty_cycle = SERVO_CCW_MAX_DUTY_R
        PWM_L.duty_cycle = SERVO_CCW_MAX_DUTY_L  
        print("Servo: Rotating Counterclockwise")
        time.sleep(5)

except KeyboardInterrupt:
    print("\nUser interrupted. Stopping servo.")

except Exception as e:
    print(f"Error: {e}")

finally:
    print("Disabling PWM and cleaning up...")
    PWM_R.duty_cycle = 00  # Set to Stop before disabling
    PWM_R.close()

    PWM_L.duty_cycle = 00  # Set to Stop before disabling
    PWM_L.close()
