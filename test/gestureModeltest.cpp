/*
 * Test code to test the reading of gesture data, training, saving and testing the gesture recognition model:
 * 1. Read the gesture data from the folder "gestureData" using the helper function
 * 2. Split the data into training and test data randomly
 * 3. Train the model using the training data
 * 4. Save the model to a file
 * 5. Load the model from the file
 * 6. Test the model using the testing data
 * 7. Print the accuracy of the model
 */

#include "../src/gesture/readGestureDataset.hpp"
#include "../src/gesture/gestureModel.hpp"
#include <random>

int main() {

	// Read the gesture data from the folder "gestureData" using the helper function
	std::vector<std::pair<std::vector<float>, int>> data;
	auto dataset = readGestureDataset("./gestureData");
	data = dataset.first;

	// Save the categories to a file
	std::ofstream category_file;
	category_file.open("categories.txt");
	for (int i = 0; i < dataset.second.size(); i++) {
		category_file << dataset.second[i] << std::endl;
	}
	category_file.close();

	std::cout << "Shape of the data: " << data.size() << ", " << data[0].first.size() << std::endl;

	// Split the data into training and test data randomly
	std::vector<std::vector<float>> x_train, x_test;
	std::vector<int> y_train, y_test;
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(data.begin(), data.end(), g);
	for (int i = 0; i < data.size(); i++) {
		if (i < data.size() * 0.8) {
			x_train.push_back(data[i].first);
			y_train.push_back(data[i].second);
		}
		else {
			x_test.push_back(data[i].first);
			y_test.push_back(data[i].second);
		}
	}

	std::cout << "Training data: " << x_train.size() << " samples" << std::endl;
	std::cout << "Test data: " << x_test.size() << " samples" << std::endl;

	// Train the model using the training data
	Net net;
	try {
		net = train_model(x_train, y_train, 10000, true, 90, true);
	} catch(std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		exit(1);
	}

	std::cout << "Model trained" << std::endl;

	// Save the model to a file
	try {
		torch::save(net, "gestureModel.pt");
	} catch (std::exception& e) {
		std::cout << "Error saving model: " << e.what() << std::endl;
		exit(1);
	}

	std::cout << "Model saved" << std::endl;

	Net net2;

	// Load the model from the file
	try {
		torch::load(net2, "gestureModel.pt");
	} catch (std::exception& e) {
		std::cout << "Error loading model: " << e.what() << std::endl;
		exit(1);
	}
	
	std::cout << "Model loaded" << std::endl;

	// Test the model using the testing data
	float acc;
	try {
		acc = accuracy(net2, x_test, y_test);
	} catch (std::exception& e) {
		std::cout << "Error testing model: " << e.what() << std::endl;
		exit(1);
	}
	std::cout << "Accuracy: " << std::setprecision(4) << acc*100 << "%" << std::endl;

	return 0;
}