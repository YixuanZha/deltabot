#include <iostream>
#include <cstdlib>
#include "a1lidarrpi.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./pwm <pulse_width_ns>" << std::endl;
        return 1;
    }

    int pulse_width_ns = atoi(argv[1]);

    if (pulse_width_ns > 2000000) pulse_width_ns = 2000000;

    A1Lidar::setPWMDutyCycle(pulse_width_ns);
    std::cout << "PWM set to pulse width: " << pulse_width_ns << " ns\n";
    std::cout << "Press Enter to disable PWM...\n";
    getchar();

    A1Lidar::disablePWM();
    return 0;
}

