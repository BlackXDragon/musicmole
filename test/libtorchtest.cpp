#include <torch/torch.h>
#include <chrono>

// Define a new Module.
struct NetImpl : torch::nn::Module {
  NetImpl() {
    // Construct and register two Linear submodules.
    fc1 = register_module("fc1", torch::nn::Linear(784, 64));
    fc2 = register_module("fc2", torch::nn::Linear(64, 32));
    fc3 = register_module("fc3", torch::nn::Linear(32, 10));
  }

  // Implement the Net's algorithm.
  torch::Tensor forward(torch::Tensor x) {
    // Use one of many tensor manipulation functions.
    x = torch::relu(fc1->forward(x.reshape({x.size(0), 784})));
    x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
    x = torch::relu(fc2->forward(x));
    x = torch::log_softmax(fc3->forward(x), /*dim=*/1);
    return x;
  }

  // Use one of many "standard library" modules.
  torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
};

TORCH_MODULE(Net);

int main() {
  // Create a new Net.
  Net net = Net();

  // If GPU is available, move the Net to the GPU.
  if (torch::cuda::is_available()) {
    net->to(torch::kCUDA);
    std::cout << "CUDA is available" << std::endl;
  }

  // Print status
  std::cout << "Net created" << std::endl;

  // Create a multi-threaded data loader for the MNIST dataset.
  auto dataset = torch::data::datasets::MNIST("./MNISTdata").map(torch::data::transforms::Stack<>());
  auto data_loader = torch::data::make_data_loader(dataset,
      /*batch_size=*/256);

  // Print status
  std::cout << "Data loader created" << std::endl;

  // Instantiate an AdamW optimization algorithm to update our Net's parameters.
  torch::optim::AdamW optimizer = torch::optim::AdamW(net->parameters());

  // Print status
  std::cout << "Optimizer created" << std::endl;

  // Start time
  auto start = std::chrono::system_clock::now();

  for (size_t epoch = 1; epoch <= 100; ++epoch) {
    size_t batch_index = 0;
    // Iterate the data loader to yield batches from the dataset.
    for (auto& batch : *data_loader) {
      // Reset gradients.
      optimizer.zero_grad();
      // Execute the model on the input data.
      // If cuda is available, move the input tensor to the GPU.
      torch::Tensor prediction;
      if(torch::cuda::is_available()) {
        prediction = net->forward(batch.data.to(torch::kCUDA));
      } else {
        prediction = net->forward(batch.data);
      }
      // Compute a loss value to judge the prediction of our model.
      torch::Tensor loss;
      if(torch::cuda::is_available())
        loss = torch::nll_loss(prediction, batch.target.to(torch::kCUDA));
      else {
        prediction.to(torch::kCPU);
        loss = torch::nll_loss(prediction, batch.target);
      }
      // Compute gradients of the loss w.r.t. the parameters of our model.
      loss.backward();
      // Update the parameters based on the calculated gradients.
      optimizer.step();
      // Output the loss and checkpoint every 100 batches.
      batch_index++;
      int n_batches = dataset.size().value() / 256;
      std::string bar = "";
      float perc_done = (float)batch_index / (float)n_batches;
      for (int i = 0; i < 20; i++) {
        if (i < perc_done * 20) {
          bar += "=";
        } else {
          bar += " ";
        }
      }
      std::cout << "Epoch: " << epoch << "\t|" << bar
                << "| " << perc_done * 100 << "% Loss: " << loss.item<float>() << "\r";
      // Serialize your model periodically as a checkpoint.
      torch::save(net, "net.pt");
    }
    std::cout << std::endl;
  }

  // End time and print duration
  auto end = std::chrono::system_clock::now();
  std::cout << "Training completed in: "
            << std::chrono::duration_cast<std::chrono::seconds>(end - start)
                   .count()
            << " s" << std::endl;

  // Load the model back from the checkpoint.
  Net net2;
  torch::load(net2, "net.pt");

  // Test the trained model.
  torch::NoGradGuard no_grad;
  net2->eval();
  size_t correct = 0;
  for (auto& batch : *data_loader) {
    // Execute the model on the input data.
    torch::Tensor prediction = net2->forward(batch.data);
    // Get the index of the max log-probability.
    correct +=
        (prediction.argmax(/*dim=*/-1) == batch.target).sum().item<int64_t>();
  }
  std::cout << "Test Accuracy: " << correct / 60000.0 << std::endl;
}