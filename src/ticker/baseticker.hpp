/*
 * Filename: baseticker.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class implementing the core features of a ticker.
 */

#if !defined(BASETICKER_HPP)
#define BASETICKER_HPP

#include <iostream>
#include <thread>
#include <functional>

// Implements a BaseTicker class
class BaseTicker {
public:
	// Basic constructor setting the callback function to be called on every tick and creates a thread that runs infinitely while calling the virtual 'run' function until the ticker is stopped.
	BaseTicker(std::function<void()> callback) : m_callback(callback) {
		m_thread = std::thread(&BaseTicker::m_run, this);
	}

	// Destructor of ticker object calls the stop function to terminate the thread.
	~BaseTicker() {
		stop();
	}

	// The stop function that instructs the thread to stop and then waits for the thread to exit.
	void stop() {
		m_running = false;
		m_stop = true;
		m_thread.join();
	}

	// The virtual 'run' function to be implemented in the derived class which executes the required logic of the type of ticker and calls the callback function.
	virtual void run() = 0;

	// The base 'm_run' function is instantiated as a thread that runs until the internal 'm_stop' is false and 'm_running' is true and calls the 'run' virtual function.
	void m_run() {
		while (!m_stop) {
			while (m_running) {
				run();
			}
		}
	}

	// The 'start' function sets 'm_running' to true causing the thread to call the virtual 'run' function.
	void start() {
		m_running = true;
	}

	// The 'pause' function sets 'm_running' to false causing the thread to run without calling the virtual 'run' function.
	void pause() {
		m_running = false;
	}

	// Retuns whether or not the ticker is running.
	bool isRunning() {
		return m_running;
	}

protected:
	std::function<void()> m_callback;

private:
	std::thread m_thread;
	std::atomic_bool m_running = false;
	std::atomic_bool m_stop = false;
};

#endif // BASETICKER_HPP