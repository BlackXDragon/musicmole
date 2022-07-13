#if !defined(BASETICKER_HPP)
#define BASETICKER_HPP

#include <iostream>
#include <thread>
#include <functional>

class BaseTicker {
public:
	BaseTicker(std::function<void()> callback) : m_callback(callback) {
		m_thread = std::thread(&BaseTicker::m_run, this);
	}

	~BaseTicker() {
		stop();
	}

	void stop() {
		m_running = false;
		m_stop = true;
		m_thread.join();
	}

	virtual void run() = 0;

	void m_run() {
		while (!m_stop) {
			while (m_running) {
				run();
			}
		}
	}

	void start() {
		m_running = true;
	}

	void pause() {
		m_running = false;
	}

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