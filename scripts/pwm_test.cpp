#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <csignal>
#include <atomic>

// Define PWM chip and channel for both servos
#define PWM_CHIP_RIGHT "pwmchip14"
#define PWM_CHANNEL_RIGHT "pwm0"
#define PWM_PATH_RIGHT "/sys/class/pwm/" PWM_CHIP_RIGHT "/" PWM_CHANNEL_RIGHT "/"

#define PWM_CHIP_LEFT "pwmchip8"
#define PWM_CHANNEL_LEFT "pwm0"
#define PWM_PATH_LEFT "/sys/class/pwm/" PWM_CHIP_LEFT "/" PWM_CHANNEL_LEFT "/"

// Atomic flag to stop the program
std::atomic<bool> running(true);

// Signal Handler
void onSignal(int) {
    running = false;
}

// Setup termination signals
void setupSignalHandlder() {
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);
    std::signal(SIGQUIT, onSignal);
    std::signal(SIGHUP, onSignal);
}

// Function to set inverted PWM duty cycle
void setPWM(const std::string &pwm_path, int pulse_width_ns) {
    std::ofstream file;

    int period = 20000000 + pulse_width_ns; // 20ms Low time, pulse_width_ns High time
    int inverted_pulse = period - pulse_width_ns; // Invert the duty cycle for Rock5B

    // Set period first
    file.open(pwm_path + "period");
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open " << pwm_path << "period" << std::endl;
        return;
    }
    file << period;
    file.flush();
    file.close();

    file.open(pwm_path + "duty_cycle");
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open " << pwm_path << "duty_cycle" << std::endl;
        return;
    }
    file << inverted_pulse;
    file.flush();
    file.close();

    // Enable PWM
    file.open(pwm_path + "enable");
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open " << pwm_path << "enable" << std::endl;
        return;
    }
    file << "1";
    file.flush();
    file.close();

    // Short delay to ensure sysfs writes are applied
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

int main() {
    std::cout << "Starting PWM test on Rock5B..." << std::endl;
    setupSignalHandlder();

    while(running) {
        // Stop position (Neutral: 7.5% duty cycle → 1.5ms pulse)
        setPWM(PWM_PATH_RIGHT, 0); // 
        setPWM(PWM_PATH_LEFT, 0);  // 
        std::cout << "PWM: Stopped (Neutral)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Rotate Clockwise (6.5% duty cycle → 1.3ms pulse)
        setPWM(PWM_PATH_RIGHT, 1300000); // 
        setPWM(PWM_PATH_LEFT, 1700000);  //
        std::cout << "PWM: Rotating Clockwise" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Stop position again (Neutral)
        setPWM(PWM_PATH_RIGHT, 0); // 
        setPWM(PWM_PATH_LEFT, 0);  // 
        std::cout << "PWM: Stopped (Neutral)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Rotate Counterclockwise (8.5% duty cycle → 1.7ms pulse)
        setPWM(PWM_PATH_RIGHT, 1700000); //
        setPWM(PWM_PATH_LEFT, 1300000);  // 
        std::cout << "PWM: Rotating Counterclockwise" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    // Stop PWM before exiting
    setPWM(PWM_PATH_RIGHT, 0); // 
    setPWM(PWM_PATH_LEFT, 0);  // 
    
    std::cout << "PWM Test Completed." << std::endl;
    return 0;
}
