#include "a1lidarrpi.h"
#include <iostream>

class DataInterface : public A1Lidar::DataInterface {
public:
	void newScanAvail(float rpm, A1LidarData (&data)[A1Lidar::nDistance]) override {
		for (A1LidarData &d : data) {
			if (d.valid) {
				printf("%e\t%e\t%e\t%e\t%e\n",
				       d.x,
				       d.y,
				       d.r,
				       d.phi,
				       d.signal_strength);
			}
		}
		fprintf(stderr, ".");
	}
};

int main(int, char **) {
	fprintf(stderr, "Data format: x <tab> y <tab> r <tab> phi <tab> strength\n");
	fprintf(stderr, "Press any key to stop.\n");

	A1Lidar lidar;
	DataInterface dataInterface;
	lidar.registerInterface(&dataInterface);

	try {
		lidar.start("/dev/ttyS2");
	} catch (const char* msg) {
		std::cerr << "ERROR: " << msg << std::endl;
		lidar.stop(); // Make sure motor and scan stop
		return 1;
	}

	// Wait for user key press
	std::cin.get();

	lidar.stop();  // Full cleanup: scan stop, motor stop, PWM off
	fprintf(stderr, "\nStopped cleanly.\n");
	return 0;
}
