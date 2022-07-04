/*
 * Test for PeriodicTicker
 */

#include "../src/ticker/ticker"

// Store last callback time.
std::chrono::steady_clock::time_point last_callback_time;

// Store sum of callback times.
std::chrono::duration<double> total_callback_time;

// Store number of callbacks.
int num_callbacks = 0;

void callback() {
	auto now = std::chrono::high_resolution_clock::now();
	auto callback_time = now - last_callback_time;
	last_callback_time = now;
	total_callback_time += callback_time;
	num_callbacks++;
	std::cout << "Callback time: " << std::chrono::duration_cast<std::chrono::milliseconds>(callback_time).count() << " ms" << std::endl;
}

int main() {
	// Create a periodic ticker.
	std::unique_ptr<BaseTicker> ticker = std::make_unique<PeriodicTicker>(1000, &callback);

	// Reset last callback time.
	last_callback_time = std::chrono::high_resolution_clock::now();

	// Start the ticker.
	ticker->start();

	// Idle until number of callbacks is greater than 10.
	while (num_callbacks < 10) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// Stop the ticker.
	ticker->stop();

	// Print average callback time.
	std::cout << "Average callback time: " << std::chrono::duration_cast<std::chrono::milliseconds>(total_callback_time).count() / num_callbacks << " ms" << std::endl;

	return 0;
}