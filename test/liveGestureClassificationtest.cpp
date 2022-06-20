/*
 * Test for classification of gestures live from sensor data
 */

#include "../src/gloveInterface/getCurrentData.hpp"
#include "../src/gesture/gestureModel.hpp"
#include <signal.h>

serialib serial;

void SIGINThandler(int sig) {
	serial.closeDevice();
	exit(0);
}

int main(int argc, char* argv[]) {
	// Check if 1 argument is passed
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <COM port>" << std::endl;
		return 1;
	}

	// Try opening serial port
	char erroropening;
	try {
		erroropening = serial.openDevice(argv[1], 9600);
	} catch (std::exception& e) {
		std::cout << "Error opening serial port: " << e.what() << std::endl;
		return 1;
	}

	if (erroropening != 1) {
		std::cout << "Error opening serial port" << std::endl;
		return erroropening;
	}

	// Set signal handler
	signal(SIGINT, SIGINThandler);

	// Load gesture model
	Net net;
	try{
		torch::load(net, "gestureModel.pt");
	} catch (std::exception& e) {
		std::cout << "Error loading model: " << e.what() << std::endl;
		return 1;
	}

	// Read the categories from a file
	std::vector<std::string> categories;
	std::ifstream categories_file("./categories.txt");
	std::string line;
	while (std::getline(categories_file, line)) {
		categories.push_back(line);
	}
	categories_file.close();

	while (true) {
		std::vector<double> data;
		auto start = std::chrono::system_clock::now();
		try {
			data = getCurrentData(serial);
		} catch (std::exception& e) {
			std::cout << "Error getting data: " << e.what() << std::endl;
			return 1;
		}
		if (data.size() != 16)
			continue;
		std::vector<float> dataf;
		try {
			dataf = std::vector<float>(data.begin(), data.end());
		} catch (std::exception& e) {
			std::cout << "Error converting data: " << e.what() << std::endl;
			return 1;
		}
		std::string prediction;
		try {
			prediction = categories[predict(net, dataf)];
		} catch (std::exception& e) {
			std::cout << "Error predicting: " << e.what() << std::endl;
			return 1;
		}
		auto end = std::chrono::system_clock::now();
		std::cout << "\x1b[2K";
		std::cout << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " us\tPrediction: " << prediction << "\r";
	}
}