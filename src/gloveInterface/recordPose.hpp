/*
 * Filename: recordPose.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Functions record and save poses.
 */

#if !defined(RECORDPOSE_HPP)
#define RECORDPOSE_HPP

#include "getCurrentData.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

// Function to record a hand pose for a set amount of time
std::vector<std::vector<double>> recordPose(serialib &serial, int nSecs = 10) {
	// Verify the serial device is open
	if (!serial.isDeviceOpen()) {
		std::cerr << "Error: serial device not open" << std::endl;
		exit(1);
	}

	// Initialise vector of vectors for storing the data
	std::vector<std::vector<double>> data;
	
	// Get start time
	auto start = std::chrono::high_resolution_clock::now();

	// Loop for the desired time
	while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() <= (nSecs * 1000)) {
		// Get current data and add to data
		std::vector<double> currentData = getCurrentData(serial);
		data.push_back(currentData);
		// Sleep for a while to not record too many samples
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	// Return the recorded data
	return data;
}

// Function to save a pose as a CSV file
void savePose(std::vector<std::vector<double>> data, std::string filename) {
	std::ofstream file;
	file.open(filename);

	for (int i = 0; i < data.size(); i++) {
		for (int j = 0; j < data[i].size(); j++) {
			file << data[i][j] << ",";
		}
		file << std::endl;
	}

	file.close();
}

// Function to record and save a pose
void recordAndSavePose(serialib &serial, std::string filename) {
	std::vector<std::vector<double>> data = recordPose(serial);
	savePose(data, filename);
}

#endif // RECORDPOSE_HPP