#include "ServoMotorSetting.h"
#include <thread>
using namespace std;

// This program gradually increases the speed of two servo motors from -10 to 10.
// The speed is changed every second, and the final speed is printed to the console.

int main(int argc, char* argv[])
{
    ServoMotorSetting servo_right(0, 20, 1.52, 14); // 1.5ms
    ServoMotorSetting servo_left(0, 20, 1.52, 8); // 1.5ms

    int speed = -10;

    for(int i = 0; i <= 20; i++)
    {
        servo_left.ChangeLeftSpeed(speed);
        servo_right.ChangeRightSpeed(speed);
        cout << "The speed is: " << speed << endl;
        speed ++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    servo_left.StopMotor();
    servo_right.StopMotor();

    return 0;
}