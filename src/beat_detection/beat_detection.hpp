#if !defined(BEATDETECTION_HPP)
#define BEATDETECTION_HPP

#include <SFML/Audio.hpp>
#include <fftw3.h>
#include <math.h>
#include <vector>
#include <chrono>
#include <iostream>

bool processSample(const int16_t *sample, const uint64_t sampleSize, uint64_t lowFreq, uint64_t highFreq, double threshold, double global_max) {
	fftw_complex *in = new fftw_complex[sampleSize], *out = new fftw_complex[sampleSize];
	fftw_plan p;

	for (int i = 0; i < sampleSize; i++) {
		in[i][0] = sample[i];
		in[i][1] = 0;
	}

	p = fftw_plan_dft_1d(sampleSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);

	std::vector<uint16_t> beats;
	double *amps = new double[sampleSize];

	// Normalize the amplitudes by sample size, get the amplitude of the complex numbers and scale it by global max
	for (int i = 0; i < sampleSize; i++) {
		double x = out[i][0] / sampleSize;
		double y = out[i][1] / sampleSize;
		amps[i] = sqrt(x * x + y * y) / global_max;
	}

	delete[] in, out;
	
	for (int i = lowFreq; i <= highFreq; i++) {
		if (amps[i] >= threshold) {
			delete[] amps;
			return true;
		}
	}

	delete[] amps;

	return false;
}

std::vector<std::chrono::microseconds> detectBeatTimes(sf::SoundBuffer &sbuffer, std::chrono::microseconds analysisPeriod = std::chrono::microseconds(1000), double lowFreq = 60, double highFreq = 150, double threshold = 0.7, std::chrono::microseconds ignorePeriod = std::chrono::microseconds(100000)) {
	const int16_t* samples = sbuffer.getSamples();
	uint64_t nSamples = sbuffer.getSampleCount();
	sf::Time duration = sbuffer.getDuration();

	std::chrono::microseconds samplePeriod = std::chrono::microseconds(duration.asMicroseconds() / nSamples);
	uint64_t ignoreSamples = ignorePeriod.count() / samplePeriod.count();
	double sampleRate = nSamples / duration.asSeconds();
	uint64_t sampleSize = analysisPeriod.count() * nSamples / duration.asMicroseconds();
	uint64_t fNyq = sampleRate / 2;
	double sampleFreqBinSize = fNyq / sampleSize;
	uint64_t lowFreqS = floor(lowFreq / sampleFreqBinSize);
	uint64_t highFreqS = floor(highFreq / sampleFreqBinSize);

	std::cout << "Sample size: " << sampleSize
				<< "\nSample rate: " << sampleRate
				<< "\nDuration: " << duration.asSeconds() << "s"
				<< "\nNyquist frequency: " << fNyq
				<< "\nSample Frequency bin size: " << sampleFreqBinSize
				<< "\nLow freq: " << lowFreq
				<< "\nLow sample freq: " << lowFreqS
				<< "\nHigh freq: " << highFreq
				<< "\nHigh sample freq: " << highFreqS
				<< std::endl;

	if (sampleSize > nSamples)
		throw std::exception("Sample size greater than number of samples");

	if (lowFreqS > sampleSize / 2)
		throw std::exception("Lower requency limit is greater than sample nyquist frequency");

	if (highFreqS > sampleSize / 2)
		highFreqS = sampleSize / 2;
	
	fftw_complex *in = new fftw_complex[sampleSize], *out = new fftw_complex[sampleSize];
	fftw_plan p;
	double *amps = new double[sampleSize];
	double global_max = -DBL_MAX;
	
	for (int s = 0; s < nSamples; s += sampleSize) {
		for (int i = 0; i < sampleSize; i++) {
			in[i][0] = samples[i];
			in[i][1] = 0;
		}

		p = fftw_plan_dft_1d(sampleSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		std::vector<uint16_t> beats;

		// Normalize the amplitudes by sample size and get the amplitude of the complex numbers
		for (int i = 0; i < sampleSize; i++) {
			double x = out[i][0] / sampleSize;
			double y = out[i][1] / sampleSize;
			amps[i] = sqrt(x * x + y * y);
		}

		// Scale the values between 0-1
		auto max_iter = std::max_element(amps, &amps[sampleSize]);

		global_max = std::max(global_max, *max_iter);
		s += ignoreSamples;
	}
	
	delete[] in, out;
	delete[] amps;

	std::vector<std::chrono::microseconds> beat_times;

	std::cout << "Starting to process\n";

	try {
		for (int i = 0; i < nSamples; i += sampleSize) {
			if (i + sampleSize >= nSamples)
				break;
			if (processSample(&samples[i], sampleSize, lowFreqS, highFreqS, threshold, global_max)) {
				beat_times.push_back(samplePeriod * i);
				i += ignoreSamples;
			}
		}
	} catch(std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	return beat_times;
}

#endif // BEATDETECTION_HPP
