/*
 * Filename: beat_detection.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Function for detecting the beats in music.
 */

#if !defined(BEATDETECTION_HPP)
#define BEATDETECTION_HPP

#include <SFML/Audio.hpp>
#include <fftw3.h>
#include <math.h>
#include <vector>
#include <chrono>
#include <iostream>

// Uses a sample of the music, the low and high frequency limits, the threshold and the global maximum in the frequency range to check whether a beat occurs in this sample.
bool processSample(const int16_t *sample, const uint64_t sampleSize, uint64_t lowFreq, uint64_t highFreq, double threshold, double global_max) {
	// Initialise the input and output complex arrays and the fftw plan
	fftw_complex *in = new fftw_complex[sampleSize], *out = new fftw_complex[sampleSize];
	fftw_plan p;

	// Copy the sample points to the input complex array
	for (int i = 0; i < sampleSize; i++) {
		in[i][0] = sample[i];
		in[i][1] = 0;
	}

	// Create, execute and destroy a DFT 1D plan.
	p = fftw_plan_dft_1d(sampleSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p);
	fftw_destroy_plan(p);

	// Initialise an array the size of sampleSize to store the amplitudes of the FFT
	// x + y*i -> sqrt(x*x + y*y)
	double *amps = new double[sampleSize];

	// Normalize the amplitudes by sample size, get the amplitude of the complex numbers and scale it by global max
	for (int i = 0; i < sampleSize; i++) {
		double x = out[i][0] / sampleSize;
		double y = out[i][1] / sampleSize;
		amps[i] = sqrt(x * x + y * y) / global_max;
	}

	// Delete the input and output complex arrays as they are no longer needed
	delete[] in, out;
	
	// Loop through the amplitudes in the frequency range
	for (int i = lowFreq; i <= highFreq; i++) {
		// If an amplitude in the desired frequency range is greater than threshold, delete the amplitudes array and return true
		if (amps[i] >= threshold) {
			delete[] amps;
			return true;
		}
	}

	// If the function hasn't returned yet, a beat has not occured in this sample. Delete amplitudes array and return false
	delete[] amps;

	return false;
}

// Takes an SFML SoundBuffer object, periods of samples to be analysed, the low and high frequency limits, threshold, and the ignore period and returns a vector of times at which beats occur in the sound in the desired frequency range, in milliseconds.
std::vector<std::chrono::microseconds> detectBeatTimes(sf::SoundBuffer &sbuffer, std::chrono::microseconds analysisPeriod = std::chrono::microseconds(1000), double lowFreq = 60, double highFreq = 150, double threshold = 0.7, std::chrono::microseconds ignorePeriod = std::chrono::microseconds(100000)) {
	// Get the array of sample points, number of sample points and total duration from the sound buffer
	const int16_t* samples = sbuffer.getSamples();
	uint64_t nSamples = sbuffer.getSampleCount();
	sf::Time duration = sbuffer.getDuration();

	// Calculate the period of each sample point by dividing the total duration by the number of sample points
	std::chrono::microseconds samplePeriod = std::chrono::microseconds(duration.asMicroseconds() / nSamples);
	// Find the number of sample points to be ignored after each sample
	uint64_t ignoreSamples = ignorePeriod.count() / samplePeriod.count();
	// Calculate the sampling rate of the sound buffer by dividing the number of sample points by the duration in seconds
	double sampleRate = nSamples / duration.asSeconds();
	// Calculate sample size from period of analysis for each sample, number of sample points and total duration
	uint64_t sampleSize = analysisPeriod.count() * nSamples / duration.asMicroseconds();
	// Find the Nyquist frequency
	uint64_t fNyq = sampleRate / 2;
	// Find the frequency of each bin after FFT
	double sampleFreqBinSize = fNyq / sampleSize;
	// Calculate the low and high frequency limits in sample domain
	uint64_t lowFreqS = floor(lowFreq / sampleFreqBinSize);
	uint64_t highFreqS = floor(highFreq / sampleFreqBinSize);

	// Printing out the calculated values to terminal for debugging
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

	// If sample size is greater than the total number of samples, throw an exception
	if (sampleSize > nSamples)
		throw std::exception("Sample size greater than number of samples");

	// If either frequency limit is greater than the sample nyquist frequency, throw an exception
	if (lowFreqS > sampleSize / 2)
		throw std::exception("Lower frequency limit is greater than sample nyquist frequency");

	if (highFreqS > sampleSize / 2)
		throw std::exception("Higher frequency limit is greater than sample nyquist frequency");
	
	// Calculate the global maximum of the frequency range

	// Initiaise the input and output complex arrays, FFTW plan, amplitudes array and the global maximum. Set global maximum to the minimum possible value of double.
	fftw_complex *in = new fftw_complex[sampleSize], *out = new fftw_complex[sampleSize];
	fftw_plan p;
	double *amps = new double[sampleSize];
	double global_max = -DBL_MAX;
	
	// Loop for the number of samplepoints skipping every sample size
	for (int s = 0; s < nSamples; s += sampleSize) {
		// Add the samples in this batch to the input complex array
		for (int i = 0; i < sampleSize; i++) {
			in[i][0] = samples[i+s];
			in[i][1] = 0;
		}

		// Create, execute and destroy the DFT 1D plan
		p = fftw_plan_dft_1d(sampleSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		// Normalize the amplitudes by sample size and get the amplitude of the complex numbers
		for (int i = 0; i < sampleSize; i++) {
			double x = out[i][0] / sampleSize;
			double y = out[i][1] / sampleSize;
			amps[i] = sqrt(x * x + y * y);
		}

		// Get the index of the max element in the amps
		auto max_iter = std::max_element(amps, &amps[sampleSize]);

		// If it is greater than the current global max, replace it
		global_max = std::max(global_max, *max_iter);
		// Skip by the number of samples to be ignored.
		s += ignoreSamples;
	}
	
	// Delete the input, output and amplitudes arrays
	delete[] in, out;
	delete[] amps;

	// Create a vector to store the times at which beats occur
	std::vector<std::chrono::microseconds> beat_times;

	// Start a try-catch block to catch potential errors
	try {
		// Loop for the number of samplepoints skipping every sample size
		for (int i = 0; i < nSamples; i += sampleSize) {
			// If the iter + sample size is greater than number of samples, we have finished, so break
			if (i + sampleSize >= nSamples)
				break;
			// Process the current sample and if a beat occured, add an entry t beat times vector
			if (processSample(&samples[i], sampleSize, lowFreqS, highFreqS, threshold, global_max)) {
				beat_times.push_back(samplePeriod * i);
			}
			// Skip the number of samples to be ignored
			i += ignoreSamples;
		}
	} catch(std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	// Return the beat times
	return beat_times;
}

#endif // BEATDETECTION_HPP
