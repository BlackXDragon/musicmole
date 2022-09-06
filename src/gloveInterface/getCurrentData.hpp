/*
 * Filename: getCurrentData.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Function to get the current data from the glove as a vector of doubles.
 */

#if !defined(GETCURRENTDATA_HPP)
#define GETCURRENTDATA_HPP

#include <serialib.h>
#include <vector>
#include <regex>
#include <iostream>

// Function to get the current data from the glove as a vector of doubles.
std::vector<double> getCurrentData(serialib &serial) {
	// Verify that the serial device is open
	if (!serial.isDeviceOpen()) {
		std::cerr << "Error: serial device not open" << std::endl;
		exit(1);
	}

	// Wait for data to be available
	while (!serial.available());

	// Create the regex pattern for the desired data format
	std::string rgxPattern = "";

	for (int i = 0; i < 16; i++) {
		rgxPattern += "([\\d\\.]+),";
	}

	rgxPattern += ".*";

	std::regex rgx(rgxPattern);

	// Read the serial port until the end of the line
	char buffer[256];
	serial.readString(buffer, '\n', 256);

	std::string str(buffer);

	// Check for the regex match
	std::smatch match;

	// If not matched, read new data till matching data is received
	while (!std::regex_search(str, match, rgx)) {
		serial.readString(buffer, '\n', 256);
		str = buffer;
	}

	// Create the data vector
	std::vector<double> data;

	for (int i = 0; i < 16; i++) {
		data.push_back(std::stod(match[i + 1]));
	}

	// Flush serial
	serial.flushReceiver();

	// Return the data
	return data;
}

#endif // GETCURRENTDATA_HPP