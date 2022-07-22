#if !defined(RECORDPOSE_HPP)
#define RECORDPOSE_HPP

#include "getCurrentData.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

std::vector<std::vector<double>> recordPose(serialib &serial, int nSecs = 10) {
	if (!serial.isDeviceOpen()) {
		std::cerr << "Error: serial device not open" << std::endl;
		exit(1);
	}

	std::vector<std::vector<double>> data;
	
	auto start = std::chrono::high_resolution_clock::now();

	while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() <= (nSecs * 1000)) {
		std::vector<double> currentData = getCurrentData(serial);
		data.push_back(currentData);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return data;
}

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

void recordAndSavePose(serialib &serial, std::string filename) {
	std::vector<std::vector<double>> data = recordPose(serial);
	savePose(data, filename);
}

#endif // RECORDPOSE_HPP