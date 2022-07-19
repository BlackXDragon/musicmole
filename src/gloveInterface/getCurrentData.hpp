#if !defined(GETCURRENTDATA_HPP)
#define GETCURRENTDATA_HPP

#include <serialib.h>
#include <vector>
#include <regex>
#include <iostream>

std::vector<double> getCurrentData(serialib &serial) {

	if (!serial.isDeviceOpen()) {
		std::cerr << "Error: serial device not open" << std::endl;
		exit(1);
	}

	while (!serial.available());

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

	std::smatch match;

	while (!std::regex_search(str, match, rgx)) {
		serial.readString(buffer, '\n', 256);
	}

	std::vector<double> data;

	for (int i = 0; i < 16; i++) {
		data.push_back(std::stod(match[i + 1]));
	}

	serial.flushReceiver();

	return data;
}

#endif // GETCURRENTDATA_HPP