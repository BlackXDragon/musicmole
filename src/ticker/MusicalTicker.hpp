#if !defined(MUSICALTICKER_HPP)
#define MUSICALTICKER_HPP

#include "baseticker.hpp"
#include <chrono>
#include <vector>
#include <SFML/Audio.hpp>
#include "../beat_detection/beat_detection.hpp"

class MusicalTicker : public BaseTicker {
public:
	MusicalTicker(std::string filename, std::function<void()> callback, std::chrono::microseconds analysisPeriod, double lowFreq, double highFreq, double threshold, std::chrono::microseconds ignorePeriod) : BaseTicker(callback), analysisPeriod(analysisPeriod), lowFreq(lowFreq), highFreq(highFreq), threshold(threshold), ignorePeriod(ignorePeriod) {
		sf::SoundBuffer buf;
		buf.loadFromFile(filename);
		this->beats = detectBeatTimes(buf, analysisPeriod, lowFreq, highFreq, threshold, ignorePeriod);

		this->music.openFromFile(filename);
	}

	void start() {
		BaseTicker::start();
		this->music.play();
	}

	void run() {
		int offset = music.getPlayingOffset().asMicroseconds();
		for (int i = 0; i < beats.size(); i++) {
			if (beats[i].count() >= offset)
				break;
			lastBeat = i;
		}
		if (offset >= music.getDuration().asMicroseconds())
			stop();
		if (offset >= beats[lastBeat].count() && offset <= (beats[lastBeat].count() + 2000)) {
			m_callback();
			lastBeat++;
		}
	}

	void stop() {
		this->music.stop();
		BaseTicker::stop();
	}

	void pause() {
		this->music.pause();
		BaseTicker::pause();
	}

private:
	std::chrono::microseconds analysisPeriod, ignorePeriod;
	std::string filename;
	double lowFreq, highFreq, threshold;
	sf::Music music;
	std::vector<std::chrono::microseconds> beats;
	int lastBeat = 0;
};

#endif // MUSICALTICKER_HPP