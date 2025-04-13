#include "a1lidarrpi.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

const std::string A1Lidar::PWM_PATH = "/sys/class/pwm/pwmchip5/pwm0/";

void A1Lidar::enablePWM() {
    std::ofstream file(PWM_PATH + "enable");
    file << "1";
    file.close();
}

void A1Lidar::disablePWM() {
    std::ofstream file(PWM_PATH + "enable");
    file << "0";
    file.close();
}

void A1Lidar::setPWMDutyCycle(int pulse_width_ns) {
    std::ofstream file;

    int period = 20000000 + pulse_width_ns;
    int inverted_pulse = period - pulse_width_ns;

    file.open(PWM_PATH + "period");
    file << period;
    file.close();

    file.open(PWM_PATH + "duty_cycle");
    file << inverted_pulse;
    file.close();

    enablePWM();
}

void A1Lidar::stop() {
	running = false;

	if (worker != nullptr) {
		worker->join();
		delete worker;
		worker = nullptr;
	}

	// Try stopping via SDK (won't work if serial is down)
	if (drv != nullptr) {
		drv->stop();
		drv->stopMotor();
		RPlidarDriver::DisposeDriver(drv);
		drv = nullptr;
	}

	// FORCE STOP MOTOR with a low PWM pulse
	setPWMDutyCycle(0);  // 1.0 ms = lowest duty = stop
	enablePWM();                // re-enable to apply the stop signal
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	disablePWM();               // optional: disable pin after delay
}



void A1Lidar::start(const char *serial_port, const unsigned rpm) {
    if (worker != nullptr) return;

    desiredRPM = (float)rpm;

    // Initial PWM setup (neutral pulse, e.g., 1.5ms to start the motor gently)
    updateMotorPWM(1500000);

    drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!drv) throw "Insufficient memory for driver";

    if (IS_OK(drv->connect(serial_port, 115200))) {
        rplidar_response_device_info_t devinfo;
        if (!IS_OK(drv->getDeviceInfo(devinfo))) {
            delete drv;
            drv = nullptr;
            disablePWM();
            throw "Cannot retrieve device info";
        }
    } else {
        disablePWM();
        throw "Cannot connect to the device";
    }

    rplidar_response_device_health_t healthinfo;
    if (!IS_OK(drv->getHealth(healthinfo))) {
        throw "Cannot retrieve device health info";
    }

    drv->startScan(0, true, 0, &scanMode);

    running = true;
    worker = new std::thread(A1Lidar::run, this);
}

void A1Lidar::updateMotorPWM(int pulse_width_ns) {
    // Clamp PWM to valid servo pulse range (~1ms to 2ms pulse)
    const int MIN_PULSE = 1000000; // 1 ms
    const int MAX_PULSE = 2000000; // 2 ms

    if (pulse_width_ns < MIN_PULSE) pulse_width_ns = MIN_PULSE;
    if (pulse_width_ns > MAX_PULSE) pulse_width_ns = MAX_PULSE;

    setPWMDutyCycle(pulse_width_ns);
}


void A1Lidar::getData() {
	size_t count = (size_t)nDistance;
	rplidar_response_measurement_node_hq_t nodes[count];
	u_result op_result = drv->grabScanDataHq(nodes, count);
	if (IS_OK(op_result)) {
		unsigned long timeNow = getTimeMS();
		if (previousTime > 0) {
			float t = (timeNow - previousTime) / 1000.0f;
			currentRPM = 1.0f/t * 60.0f;
		}
		previousTime = timeNow;
		drv->ascendScanData(nodes, count);
		for (int pos = 0; pos < (int)count ; ++pos) {
			float angle = M_PI - nodes[pos].angle_z_q14 * (90.f / 16384.f / (180.0f / M_PI));
			float dist = nodes[pos].dist_mm_q2/4000.0f;
			if (dist > 0) {
				//fprintf(stderr,"%d,phi=%f,r=%f\n",j,angle,dist);
				a1LidarData[currentBufIdx][pos].phi = angle;
				a1LidarData[currentBufIdx][pos].r = dist;
				a1LidarData[currentBufIdx][pos].x = cos(angle) * dist;
				a1LidarData[currentBufIdx][pos].y = sin(angle) * dist;
				a1LidarData[currentBufIdx][pos].signal_strength =
					nodes[pos].quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;
				a1LidarData[currentBufIdx][pos].valid = true;
				dataAvailable = true;
			} else {
				a1LidarData[currentBufIdx][pos].valid = false;
			}
		}
		updateMotorPWM(
			       motorDrive +
			       (int)round((desiredRPM - currentRPM) * loopRPMgain * (float)pwmRange)
			       );
		if ( (dataAvailable) && (nullptr != dataInterface) ) {
			dataInterface->newScanAvail(currentRPM, a1LidarData[currentBufIdx]);
		}
		readoutMtx.lock();
		currentBufIdx = !currentBufIdx;
		readoutMtx.unlock();
	}
}

void A1Lidar::run(A1Lidar* a1Lidar) {
	while (a1Lidar->running) {
		a1Lidar->getData();
	}
	a1Lidar->updateMotorPWM(0);
	a1Lidar->setPWMDutyCycle(1500000);
}
