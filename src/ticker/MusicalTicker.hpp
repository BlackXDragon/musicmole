/*
 * Filename: MusicalTicker.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class deriving from the BaseTicker which calls the callback function when a beat occurs in a specified music file.
 */

#if !defined(MUSICALTICKER_HPP)
#define MUSICALTICKER_HPP

#include "baseticker.hpp"
#include <chrono>
#include <vector>
#include <SFML/Audio.hpp>
#include "../beat_detection/beat_detection.hpp"
#include <future>

// MusicalTicker class deriving from the BaseTicker class that calls the callback function when a beat occurs in a specified music file.
class MusicalTicker : public BaseTicker {
public:
	// Constructor accepts path of the music file, callback function, and the parameters required for beat detection. Reads the music file, performs beat detection and stores the times at which beats occur.
	MusicalTicker(std::string filename, std::function<void()> callback, std::chrono::microseconds analysisPeriod, double lowFreq, double highFreq, double threshold, std::chrono::microseconds ignorePeriod) : BaseTicker(callback), analysisPeriod(analysisPeriod), lowFreq(lowFreq), highFreq(highFreq), threshold(threshold), ignorePeriod(ignorePeriod) {
		sf::SoundBuffer buf;
		buf.loadFromFile(filename);
		this->beats = detectBeatTimes(buf, analysisPeriod, lowFreq, highFreq, threshold, ignorePeriod);

		this->music.openFromFile(filename);
	}

	// 'Start' function overrided for starting the music
	void start() {
		BaseTicker::start();
		this->music.play();
	}

	// Implementation of the virtual 'run' function the gets the current offset of the music playing, checks if a beat is currently occuring and calls the callback function if true.
	void run() {
		// Retrieve the offset of the currently playing music in microseconds.
		int offset = music.getPlayingOffset().asMicroseconds();
		
		// Check if the music has ended by checking if the offset is greater or equal to the maximum song duration or the status of music is stopped. If so, stop the ticker.
		if (offset >= music.getDuration().asMicroseconds() || music.getStatus() == sf::SoundSource::Stopped)
			stop();
		
		// Check whether the last beat time has passed, and if so, update the lastBeat counter.
		bool e = false;
		int max_beats = beats.size();
		while (beats[lastBeat].count() <= offset) {
			lastBeat = lastBeat + 1 % max_beats;
			if (music.getStatus() == sf::SoundSource::Stopped)
				break;
			e = true;
		}
		lastBeat -= e;

		// If the current offset is greater than last beat time and less than 2000 microseconds after the last beat time.
		auto b = beats[lastBeat].count();
		if (offset >= b && offset <= (b + 2000)) {
			auto ret = std::async(std::launch::async, m_callback);
			lastBeat++;
		}
	}

	// 'Stop' function overrided for stopping the music
	void stop() {
		BaseTicker::stop();
		this->music.stop();
	}

	// 'Pause' function overrided for pausing the music
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