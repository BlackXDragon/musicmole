#if !defined(MUSICALTICKER_HPP)
#define MUSICALTICKER_HPP

#include "baseticker.hpp"
#include <chrono>
#include <vector>
#include <SFML/Audio.hpp>
#include "../beat_detection/beat_detection.hpp"
#include <future>

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
		if (offset >= music.getDuration().asMicroseconds() || music.getStatus() == sf::SoundSource::Stopped)
			stop();
		bool e = false;
		int max_beats = beats.size();
		while (beats[lastBeat].count() <= offset) {
			lastBeat = lastBeat + 1 % max_beats;
			if (music.getStatus() == sf::SoundSource::Stopped)
				break;
			e = true;
		}
		lastBeat -= e;
		auto b = beats[lastBeat].count();
		if (offset >= b && offset <= (b + 2000)) {
			auto ret = std::async(std::launch::async, m_callback);
			lastBeat++;
		}
	}

	void stop() {
		BaseTicker::stop();
		this->music.stop();
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