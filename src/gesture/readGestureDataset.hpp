/*
 * Filename: readGestureDataset.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Function to read gesture datasets (stored as .csv files in a single folder) as vector of vectors of floats and the categories as a vector of ints.
 */

#if !defined(READ_GESTURE_DATASET_HPP)
#define READ_GESTURE_DATASET_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// Reads a gesture dataset from a folder and returns a vector of vectors of floats and a vector of ints.
std::pair<std::vector<std::pair<std::vector<float>, int>>, std::vector<std::string>> readGestureDataset(std::string folder_path) {
	std::vector<std::pair<std::vector<float>, int>> data;

	// Get list of files in the folder.
	std::vector<std::string> files;
	for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
		if (entry.path().extension() == ".csv") {
			files.push_back(entry.path().string());
		}
	}

	std::vector<std::string> categories;

	// Read each file.
	for (int i = 0; i < files.size(); i++) {
		std::ifstream file(files[i]);
		std::string line;
		int y_i;
		while (std::getline(file, line)) {
			if (line == "")
				continue;
			std::stringstream ss(line);
			std::string token;
			std::vector<float> x_i;
			int c = 0;
			while (std::getline(ss, token, ',') && c < 16) {
				x_i.push_back(std::stof(token));
				c++;
			}
			data.push_back(std::make_pair(x_i, i));
		}
		categories.push_back(files[i].substr(folder_path.size() + 1, files[i].size() - folder_path.size() - 5));
	}

	return std::make_pair(data, categories);
}

#endif // READ_GESTURE_DATASET_HPP