/*
 * Helper functions to deal with training, saving, loading and testing of the multi-class gesture recognition model.
 */

#if !defined(GestureModel_HPP)
#define GestureModel_HPP

#include <dlib/svm_threaded.h>
#include <dlib/rand.h>
#include "readGestureDataset.hpp"

// Define sample type and trainer type.
typedef dlib::matrix<float, 16, 1> sample_type;
typedef dlib::svm_multiclass_linear_trainer<dlib::linear_kernel<sample_type>> trainer_t;
typedef dlib::multiclass_linear_decision_function<dlib::linear_kernel<sample_type>, float> df_t;

// Function to create the gesture recognition model.
trainer_t createGestureModel() {
	trainer_t trainer;
	// trainer.set_kernel(dlib::radial_basis_kernel<sample_type>(0.1));
	trainer.set_c(1);
	trainer.set_epsilon(0.1);
	return trainer;
}

// Function to convert a vector of floats to a sample type.
sample_type convertToSampleType(std::vector<float> sample) {
	sample_type sampleType;
	for (int i = 0; i < sample.size(); i++) {
		sampleType(i) = sample[i];
	}
	return sampleType;
}

// Function to convert a vector of vector of floats to a vector of sample types.
std::vector<sample_type> convertToSampleTypes(std::vector<std::vector<float>> data) {
	std::vector<sample_type> samples;
	for (int i = 0; i < data.size(); i++) {
		sample_type sample;
		for (int j = 0; j < data[i].size(); j++) {
			sample(j) = data[i][j];
		}
		samples.push_back(sample);
	}
	return samples;
}

// Function to train the gesture recognition model and return the decision function.
df_t trainGestureModel(trainer_t& trainer, std::vector<sample_type>& samples, std::vector<float>& labels) {
	dlib::randomize_samples(samples, labels);
	trainer.set_max_iterations(1000);
	// trainer.set_epsilon_convergence(0.001);
	trainer.set_c(1);
	// trainer.set_kernel(dlib::radial_basis_kernel<sample_type>(0.1));
	trainer.set_epsilon(0.1);
	return trainer.train(samples, labels);
}

// Function to save the gesture recognition model.
void saveGestureModel(df_t& trainer, std::string filename) {
	dlib::serialize(filename) << trainer;
}

// Function to load the gesture recognition model.
void loadGestureModel(df_t& trainer, std::string filename) {
	dlib::deserialize(filename) >> trainer;
}

// Function to calculate the accuracy of the gesture recognition model.
float calculateAccuracy(df_t& trainer, std::vector<sample_type>& samples, std::vector<float>& labels) {
	int correct = 0;
	for (int i = 0; i < samples.size(); i++) {
		if (trainer(samples[i]) == labels[i]) {
			correct++;
		}
	}
	return (float)correct / (float)samples.size();
}

#endif // GestureModel_HPP