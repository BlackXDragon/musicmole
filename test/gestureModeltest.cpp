/*
 * Function to test the gesture recognition model.
 */

#include <iostream>
#include <random>
#include "../src/gesture/readGestureDataset.hpp"
#include "../src/gesture/gestureModel.hpp"

int main() {
	// Read in the gesture dataset.
	auto dataset = readGestureDataset("./gestureData");

	// Print status.
	std::cout << "Read in " << dataset.first.size() << " gestures." << std::endl;

	auto data = dataset.first;
	auto categories = dataset.second;

	// Extract the samples and labels.
	std::vector<std::vector<float>> samples;
	std::vector<float> labels;
	for (int i = 0; i < data.size(); i++) {
		samples.push_back(data[i].first);
		labels.push_back(data[i].second);
	}

	// Save the categories to a file.
	std::ofstream categoriesFile;
	categoriesFile.open("./categories.txt");
	for (int i = 0; i < categories.size(); i++) {
		categoriesFile << categories[i] << std::endl;
	}
	categoriesFile.close();

	// Convert samples to a vector of sample types.
	std::vector<sample_type> sampleTypes = convertToSampleTypes(samples);

	// Print status.
	std::cout << "Converted " << samples.size() << " samples to sample types." << std::endl;

	// Split the dataset into training (80%) and testing (20%) sets randomly.
	std::vector<sample_type> trainingSamples;
	std::vector<sample_type> testingSamples;
	std::vector<float> trainingLabels;
	std::vector<float> testingLabels;

	std::random_device rd;
	std::mt19937 g(rd());
	
	std::uniform_real_distribution<> dist(0, 1);
	for (int i = 0; i < samples.size(); i++) {
		if (dist(g) < 0.8) {
			trainingSamples.push_back(sampleTypes[i]);
			trainingLabels.push_back(labels[i]);
		} else {
			testingSamples.push_back(sampleTypes[i]);
			testingLabels.push_back(labels[i]);
		}
	}

	// Print status.
	std::cout << "Split " << samples.size() << " samples into training and testing sets." << std::endl;

	// Create the gesture recognition model.
	auto trainer = createGestureModel();

	// Print status.
	std::cout << "Created the gesture recognition model." << std::endl;

	// Train the gesture recognition model.
	auto decisionFunction = trainGestureModel(trainer, trainingSamples, trainingLabels);

	// Print status.
	std::cout << "Trained the gesture recognition model." << std::endl;

	// Save the gesture recognition model.
	saveGestureModel(decisionFunction, "./gestureModel.dat");

	// Print status.
	std::cout << "Saved the gesture recognition model." << std::endl;
	
	// Create a new decision function and load the gesture recognition model.
	df_t newDecisionFunction;
	loadGestureModel(newDecisionFunction, "./gestureModel.dat");

	// Print status.
	std::cout << "Loaded the gesture recognition model." << std::endl;

	// Calculate the accuracy of the gesture recognition model.
	auto accuracy = calculateAccuracy(newDecisionFunction, testingSamples, testingLabels);

	// Print the accuracy.
	std::cout << "Accuracy: " << accuracy << std::endl;
}