/*
 * Train a gesture recognition NN model from 16 data points into n categories.
 */

#include <torch/torch.h>
#include <vector>
#include <iostream>

// Definition of the Net class.
class NetImpl : public torch::nn::Module {
public:
	NetImpl() : NetImpl(16, {8, 4}, 6) {}
	NetImpl(int n_input, std::vector<int> n_hidden, int n_output) : Module() {
		auto start = std::chrono::system_clock::now();
		n_hidden_layers = n_hidden.size();
		if (n_hidden_layers > 0) {
			fc1 = register_module("fc1", torch::nn::Linear(n_input, n_hidden[0]));
			fc1_bn = register_module("fc1_bn", torch::nn::BatchNorm1d(n_hidden[0]));

			for (int i = 0; i < n_hidden_layers - 1; i++) {
				std::stringstream ss;
				ss << "fc" << i + 2;
				auto fc = register_module(ss.str(), torch::nn::Linear(n_hidden[i], n_hidden[i + 1]));
				std::stringstream ss_bn;
				ss_bn << "fc" << i + 2 << "_bn";
				auto fc_bn = register_module(ss_bn.str(), torch::nn::BatchNorm1d(n_hidden[i + 1]));
				fc_layers.push_back(fc);
				fc_layers_bn.push_back(fc_bn);
			}

			fc_out = register_module("fc_out", torch::nn::Linear(n_hidden[n_hidden.size() - 1], n_output));
		} else {
			fc_out = register_module("fc_out", torch::nn::Linear(n_input, n_output));
		}
		auto end = std::chrono::system_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		std::cout << "Net constructor took " << duration.count() << " milliseconds." << std::endl;
	}

	torch::Tensor forward(torch::Tensor x) {
		if (n_hidden_layers > 0) {
			x = torch::relu(fc1->forward(x));
			x = fc1_bn->forward(x);
			x = torch::dropout(x, /*p=*/0.2, /*train=*/is_training());

			for (int i = 0; i < fc_layers.size(); i++) {
				try {
					x = torch::relu(fc_layers[i]->forward(x));
					x = fc_layers_bn[i]->forward(x);
					x = torch::dropout(x, /*p=*/0.2, /*train=*/is_training());
				} catch(std::exception& e) {
					std::cout << "Error: " << e.what() << std::endl;
				}
			}
		}

		x = fc_out->forward(x);
		// x = torch::log_softmax(x, /*dim=*/1);
		return x;
	}

	int n_hidden_layers = 0;
	torch::nn::Linear fc1{nullptr};
	torch::nn::BatchNorm1d fc1_bn{nullptr};
	// torch::nn::Linear fc2{nullptr};
	std::vector<torch::nn::Linear> fc_layers;
	std::vector<torch::nn::BatchNorm1d> fc_layers_bn;
	torch::nn::Linear fc_out{nullptr};
};

TORCH_MODULE(Net);

// Function to train and return the model.
Net train_model(std::vector<std::vector<float>> x, std::vector<int> y, int n_epochs = 25, bool early_stopping = false, double min_acc = 90, bool verbose = false, bool save_periodically = false, std::string save_path = "") {
	// Convert y to one-hot encoding.
	int y_min = *std::min_element(y.begin(), y.end());
	int y_max = *std::max_element(y.begin(), y.end());
	std::vector<std::vector<float>> y_one_hot;
	for (int i = 0; i < y.size(); i++) {
		std::vector<float> y_one_hot_i(y_max - y_min + 1, 0);
		y_one_hot_i[y[i] - y_min] = 1;
		y_one_hot.push_back(y_one_hot_i);
	}
	// Print y_one_hot shape
	// std::cout << "y_one_hot shape: " << y_one_hot.size() << "x" << y_one_hot[0].size() << std::endl;
	
	// Create a new Net.
	std::vector<int> hidden = { (int)(x[0].size()/2), (int)(x[0].size()/4) };

	std::cout << "Hidden layers: ";
	for (int i = 0; i < hidden.size(); i++) {
		std::cout << hidden[i] << " ";
	}
	std::cout << std::endl;

	auto net = Net(x[0].size(), hidden, y_one_hot[0].size());

	std::cout << "Model created" << std::endl;

	// If GPU is available, move the Net to the GPU.
	if (torch::cuda::is_available()) {
		net->to(torch::kCUDA);
		std::cout << "CUDA is available" << std::endl;
	}

	// Create a multi-threaded data loader for the MNIST dataset.
	auto xtensor = torch::zeros({ (int)(x.size()), (int)(x[0].size()) });
	for (int i = 0; i < x.size(); i++)
		xtensor.slice(0, i,i+1) = torch::from_blob(x[i].data(), torch::IntArrayRef({ (int)(x[i].size()) }));
	// auto ytensor = torch::from_blob(y.data(), torch::IntArrayRef({ (int)(y.size()) }));
	auto ytensor = torch::zeros({ (int)(y_one_hot.size()), (int)(y_one_hot[0].size()) });
	for (int i = 0; i < y_one_hot.size(); i++)
		ytensor.slice(0, i,i+1) = torch::from_blob(y_one_hot[i].data(), torch::IntArrayRef({ (int)(y_one_hot[i].size()) }));
	// ytensor = ytensor.transpose(1, 0);
	// // Print y sizes
	// std::cout << "y tensor sizes: ";
	// for (int i: ytensor.sizes()) {
	// 	std::cout << i << " ";
	// }
	// std::cout << std::endl;

	std::cout << "Data loaded to tensors" << std::endl;

	if (torch::cuda::is_available()) {
		xtensor = xtensor.to(torch::kCUDA);
		ytensor = ytensor.to(torch::kCUDA);
	}

	// Instantiate an AdamW optimization algorithm to update our Net's parameters.
	torch::optim::AdamWOptions optimOptions = torch::optim::AdamWOptions(0.02)
												.weight_decay(0.001);
	torch::optim::AdamW optimizer = torch::optim::AdamW(net->parameters(), optimOptions);
	
	std::cout << "Optimizer created" << std::endl;

	torch::nn::CrossEntropyLoss criterion = torch::nn::CrossEntropyLoss();

	// Start time
	auto start = std::chrono::system_clock::now();

	// Train the model.
	for (size_t epoch = 1; epoch <= n_epochs; ++epoch) {
		// Reset gradients.
		optimizer.zero_grad();

		// std::cout << "Optimizer reset" << std::endl;

		// Execute the model on the input data.
		torch::Tensor prediction;
		prediction = net->forward(xtensor);

		// std::cout << "Forward pass done" << std::endl;

		// std::cout << "Prediction is in CUDA? " << prediction.is_cuda() << std::endl;
		// std::cout << "Y is in CUDA? " << ytensor.is_cuda() << std::endl;

		// Compute a loss value to judge the prediction of our model.
		torch::Tensor loss;
		try {
			loss = criterion(prediction, ytensor);
		} catch(std::exception& e) {
			std::cout << "Error: " << e.what() << std::endl;
			exit(1);
		}

		// std::cout << "Loss computed" << std::endl;
		
		// Compute gradients of the loss w.r.t. the parameters of our model.
		loss.backward();

		// std::cout << "Gradients computed" << std::endl;
		
		// Update the parameters based on the calculated gradients.
		optimizer.step();

		// std::cout << "Optimizer step" << std::endl;

		std::vector<int> predictions;
		for (int i = 0; i < x.size(); i++)
			predictions.push_back(prediction.slice(0, i,i+1).argmax().item<int>());
		int correct = 0;
		for (int i = 0; i < (int)(x.size()); i++)
			if (predictions[i] == y[i])
				correct++;
		double acc = (double)correct / (double)(x.size());
		
		if (verbose) {
			std::string bar = "";
			float perc_done = (float)epoch / (float)n_epochs;
			for (int i = 0; i < 20; i++) {
				if (i < perc_done * 20) {
				bar += "=";
				} else {
				bar += " ";
				}
			}
			std::cout << "Epoch: " << epoch
						// << "\t|" << bar << "| " << perc_done * 100 << "%"
						<< "\tLoss: " << loss.item<float>()
						<< "\tAccuracy: " << std::setprecision(4) << acc * 100 << "%\n";
		}

		if (early_stopping && acc*100 >= min_acc)
			break;
		
		if (save_periodically) {
			// Serialize your model periodically as a checkpoint.
			torch::save(net, save_path+"net.pt");
		}
	}

	if (verbose) {
		// End time and print duration
		auto end = std::chrono::system_clock::now();
		std::cout << "\nTraining completed in: "
					<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
						.count()
					<< " ms" << std::endl;
	}

	return net;
}

// Function to predict the class of a given input.
int predict(Net net, std::vector<float> x) {
	net->train(false);
	torch::Tensor prediction;
	if(torch::cuda::is_available()) {
		prediction = net->forward(torch::from_blob(x.data(), { 1, (int)(x.size()) }).to(torch::kCUDA));
	} else {
		prediction = net->forward(torch::from_blob(x.data(), { 1, (int)(x.size()) }));
	}
	return prediction.argmax().item<int>();
}

// Function to predict the class of a vector of inputs.
std::vector<int> predict(Net net, std::vector<std::vector<float>> x) {
	net->train(false);
	torch::Tensor predictions;
	auto xtensor = torch::zeros({ (int)(x.size()), (int)(x[0].size()) });
	for (int i = 0; i < x.size(); i++)
		xtensor.slice(0, i,i+1) = torch::from_blob(x[i].data(), torch::IntArrayRef({ (int)(x[i].size()) }));
	
	if(torch::cuda::is_available()) {
		predictions = net->forward(xtensor.to(torch::kCUDA));
	} else {
		predictions = net->forward(xtensor);
	}

	std::vector<int> predictions_vec;
	for (int i = 0; i < x.size(); i++)
		predictions_vec.push_back(predictions.slice(0, i,i+1).argmax().item<int>());
	return predictions_vec;
}

// Function to calculate the accuracy of the model.
float accuracy(Net net, std::vector<std::vector<float>> x, std::vector<int> y) {
	auto predictions = predict(net, x);
	int correct = 0;
	for (int i = 0; i < (int)(x.size()); i++)
		if (predictions[i] == y[i])
			correct++;
	return (float)correct / (float)(x.size());
}