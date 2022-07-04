#if !defined(PERIODICTICKER_HPP)
#define PERIODICTICKER_HPP

#include "baseticker.hpp"
#include <chrono>

class PeriodicTicker : public BaseTicker {
public:
	PeriodicTicker(long long millis, std::function<void()> callback) : BaseTicker(callback) {
		m_period = std::chrono::milliseconds(millis);
	}

	void run() {
		m_callback();
		std::this_thread::sleep_for(m_period);
	}

private:
	std::chrono::milliseconds m_period;
};

#endif // PERIODICTICKER_HPP