/*
 * Filename: PeriodicTicker.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class deriving from the BaseTicker which calls the callback function at specified intervals
 */

#if !defined(PERIODICTICKER_HPP)
#define PERIODICTICKER_HPP

#include "baseticker.hpp"
#include <chrono>

// The PeriodicTicker class derives from the BaseTicker class and calls the callback function at specified intervals.
class PeriodicTicker : public BaseTicker {
public:
	// The constructor accepting the period of the periodic ticker in milliseconds and the callback function.
	PeriodicTicker(long long millis, std::function<void()> callback) : BaseTicker(callback) {
		m_period = std::chrono::milliseconds(millis);
	}

	// Implementation of the virutal 'run' function which calls the callback function and then sleeps for the specified period.
	void run() {
		m_callback();
		std::this_thread::sleep_for(m_period);
	}

private:
	std::chrono::milliseconds m_period;
};

#endif // PERIODICTICKER_HPP