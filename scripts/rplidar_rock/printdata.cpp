#include "a1lidarrpi.h"
#include <iostream>
#include <fstream>  // Include for file handling
#include <thread>   // For sleep functionality
#include <chrono>   // For timing


class DataInterface : public A1Lidar::DataInterface {
public:

	void newScanAvail(float rpm, A1LidarData (&data)[A1Lidar::nDistance]) {
		// Debugging log to see if the function is triggered
		std::cerr << "newScanAvail triggered!" << std::endl;

		std::ofstream dataFile;
		dataFile.open("lidar_data.csv", std::ios_base::app);  // Open file in append mode

		for (A1LidarData &d : data) {
			if (d.valid) {  // Only process valid data
				std::cout << "x: " << d.x << "\t y: " << d.y 
						<< "\t r: " << d.r << "\t phi: " << d.phi 
						<< "\t strength: " << d.signal_strength << std::endl;

				// Save valid data to CSV file
				dataFile << d.x << "," << d.y << "," << d.r << "," << d.phi 
						<< "," << d.signal_strength << "\n";
			}
		}
		dataFile.close();
		std::cerr << ".";  // Log that data was received and saved
	}



};

int main(int, char **) {
    fprintf(stderr, "Data format: x <tab> y <tab> r <tab> phi <tab> strength\n");
    fprintf(stderr, "Press any key to stop.\n");

    A1Lidar lidar;
    DataInterface dataInterface;
    lidar.registerInterface(&dataInterface);  // Registers the callback function

    try {
        lidar.start("/dev/ttyS2");  // Start the LIDAR
    } catch (const char* msg) {
        std::cerr << "ERROR: " << msg << std::endl;
        lidar.stop(); // Make sure motor and scan stop
        return 1;
    }

    // Keep the program running while scanning (waiting for new data)
    std::cerr << "Waiting for data..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));  // Allow time for scanning

    std::cerr << "\nStopped cleanly." << std::endl;
    lidar.stop();  // Stop the scanning and motor
    return 0;
}



