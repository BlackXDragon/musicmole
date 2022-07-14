/*
 * Code to test the gesture recognition model on live data from serial.
 */

#include <iostream>
#include <signal.h>
#include "../src/gloveInterface/getCurrentData.hpp"
#include "../src/gesture/gestureModel.hpp"

serialib serial;

// SIGINT handler.
void signal_handler(int signal) {
	serial.closeDevice();
	std::cout << "Exiting..." << std::endl;
	exit(0);
}

int main(int argc, char* argv[]) {
	// Check if the user has provided a serial port.
	if (argc < 2) {
		std::cout << "Please provide a serial port." << std::endl;
		return 1;
	}
	
	// Try to open the serial port.
	char err;
	try {
		err = serial.openDevice(argv[1], 9600);
	} catch (std::exception& e) {
		std::cout << "Error opening serial port: " << e.what() << std::endl;
		return 1;
	}

	if (err != 1) {
		std::cout << "Error opening serial port: " << err << std::endl;
		return err;
	}

	// Set the SIGINT handler.
	signal(SIGINT, signal_handler);

	// Create the gesture recognition model.
	df_t df;

	// Load the gesture recognition model.
	loadGestureModel(df, "./gestureModel.dat");

	// Print status.
	std::cout << "Loaded gesture recognition model." << std::endl;

	// Load the categories from the file.
	std::vector<std::string> categories;
	std::ifstream categoriesFile;
	categoriesFile.open("./categories.txt");
	std::string line;
	while (std::getline(categoriesFile, line)) {
		categories.push_back(line);
	}
	categoriesFile.close();

	// Print status.
	std::cout << "Loaded categories." << std::endl;

	while (true) {
		auto start = std::chrono::high_resolution_clock::now();
		// Get the current data.
		std::vector<double> data;
		try {
			data = getCurrentData(serial);
		} catch (std::exception& e) {
			std::cout << "Error getting data: " << e.what() << std::endl;
			return 1;
		}
		if (data.size() == 0) {
			std::cout << "Error getting data." << "\r";
			continue;
		}
		
		// Convert the data to a sample type.
		sample_type sample;
		try {
			sample = convertToSampleType(std::vector<float>(data.begin(), data.end()));
		} catch (std::exception& e) {
			std::cout << "Error converting data: " << e.what() << std::endl;
			return 1;
		}
		
		// Predict the gesture.
		float prediction;
		try {
			prediction = df(sample);
		} catch (std::exception& e) {
			std::cout << "Error predicting gesture: " << e.what() << std::endl;
			return 1;
		}

		auto end = std::chrono::high_resolution_clock::now();
		
		// Clear the current line and print time and prediction.
		std::cout << "\033[2K\r";
		std::cout << "Prediction: " << categories[(int)prediction] << "\t Time taken: " << std::chrono::duration_cast<std::chrono::microseconds>(end-start).count() << " us\r";
	}
}