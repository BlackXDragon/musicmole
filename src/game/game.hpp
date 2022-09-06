/*
 * Filename: game.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class implementing the game logic and graphics.
 */

#if !defined(GAME_H)
#define GAME_H

#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <random>
#include <functional>
#include <thread>
#include <mutex>
#include <SFML/Graphics.hpp>
#include "Hole.hpp"
#include "Mole.hpp"

// Defines the logic of the mole game and function for displaying to the screen. Moles can be randomly spawned on the board and vanish in 6 seconds if not whacked.
class Game {
public:
	// The default 3x3 game constructor
	Game() : Game(3, 3) {}

	// Constructor with customisable board size
	Game(int x = 3, int y = 3) {
		// Set the board x and y sizes
		this->x = x;
		this->y = y;
		// Loop through the sizes and initialise the board, mole start times and terminate_remove_thread vectors
		for (int i = 0; i < x; i++) {
			std::vector<bool> b;
			std::vector<std::chrono::steady_clock::time_point> t;
			std::vector<bool> trt;
			for (int j = 0; j < y; j++) {
				b.push_back(false);
				t.push_back(std::chrono::high_resolution_clock::now());
				trt.push_back(false);
			}
			this->board.push_back(b);
			this->mole_start_times.push_back(t);
			this->terminate_remove_thread.push_back(trt);
		}

		// Initialise the number of whacked, missed and total moles and the times taken to whack
		this->n_whacked = 0;
		this->n_missed = 0;
		this->n_total = 0;
		this->whack_times = std::vector<std::chrono::duration<double>>();

		// Initialise random device and uniform distributions for x and y
		this->rd = std::mt19937(std::random_device()());
		this->xdist = std::uniform_int_distribution<int>(0, x - 1);
		this->ydist = std::uniform_int_distribution<int>(0, y - 1);

		// Load the mole and hole texture, set sprites and their scales and origins
		this->moleTex.loadFromMemory(Mole_png, Mole_png_len);
		this->holeTex.loadFromMemory(Hole_png, Hole_png_len);
		this->moleSprite.setTexture(moleTex);
		this->holeSprite.setTexture(holeTex);
		this->moleSprite.setScale(0.3, 0.3);
		this->holeSprite.setScale(0.3, 0.3);
		auto holeSize = holeSprite.getLocalBounds();
		auto moleSize = moleSprite.getLocalBounds();
		this->moleSprite.setOrigin(0.5 * moleSize.width, 0.5 * moleSize.height);
		this->holeSprite.setOrigin(0.5 * holeSize.width, 0.5 * holeSize.height);
	}

	// Reset function to reset the scoring variables
	void reset() {
		this->n_total = 0;
		this->n_whacked = 0;
		this->n_missed = 0;
		this->whack_times.clear();
	}

	// Function to create a random mole
	void make_mole() {
		// Get random x and y values until there is no mole in that location
		int x = xdist(rd);
		int y = ydist(rd);
		while (board[x][y]) {
			x = xdist(rd);
			y = ydist(rd);
		}
		
		// Set the board to have a mole there
		board[x][y] = true;

		// Check if the board is full of moles
		bool board_full = true;
		for (int i = 0; i < this->x; i++)
			for (int j = 0; j < this->y; j++)
				if (!board[i][j])
					board_full = false;
		
		// If the board is full of moles, reset the game and call the game over callback function
		if (board_full) {
			this->reset();
			this->game_over_callback();
		}

		// Set the time at which the mole is spawned
		this->mole_start_times[x][y] = std::chrono::high_resolution_clock::now();
		// Start a thread to remove the mole in 6 seconds
		m_threads.push_back(std::thread(&Game::remove_mole, this, x, y));
		// Increment the total number of moles
		n_total++;
	}

	// Function to whack a mole at a location x and y passed are from [0, x] and [0, y]
	void whack(int x, int y) {
		// Decrement x and y to get ranges of [-1, x-1] and [-1, y-1]
		x--; y--;
		if (x == -1 || y == -1) { // Condition for no input while using Glove controller
			return;
		}

		// If the board has a mole at the location
		if (board[x][y]) {
			// Acquire mutex for terminate_remove_thread
			trt_mutex.lock();
			// Set it to true to stop the thread
			terminate_remove_thread[x][y] = true;
			trt_mutex.unlock();
			// Set the board to false at that location
			board[x][y] = false;
			// Increment the number of moles whacked
			n_whacked++;
			// Add the response time to the whack times vector
			whack_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - mole_start_times[x][y]));
		}
	}

	// Function to print the current board to the terminal
	void print_board() {
		std::cout << "+---+---+---+\n";
		for (int i = 0; i < this->x; i++) {
			for (int j = 0; j < this->y; j++) {
				std::cout << "| " << (board[i][j] ? "#" : " ") << " ";
			}
			std::cout << "|\n+---+---+---+\n";
		}
	}

	// Function to get the number of moles whacked
	int get_n_whacked() {
		return n_whacked;
	}

	// Function to get the number of moles missed
	int get_n_missed() {
		return n_missed;
	}

	// Function to get the total number of moles
	int get_n_total() {
		return n_total;
	}

	// Function to get the response times vector
	std::vector<std::chrono::duration<double>> get_whack_times() {
		return whack_times;
	}

	// Function to get the average response time
	double get_avg_whack_time() {
		double sum = 0;
		for (auto time : whack_times) {
			sum += std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
		}
		return sum / whack_times.size();
	}

	// Function to get the score as the fraction of moles whacked
	double get_score() {
		return (double)n_whacked / n_total;
	}
	
	// Function to get the board
	std::vector<std::vector<bool>> get_board() {
		return board;
	}

	// Function to get the x size
	int get_x() {
		return x;
	}

	// Function to get the y size
	int get_y() {
		return y;
	}

	// Function to set the callback function for when the board becomes full
	void set_game_over_callback(std::function<void()> f) {
		this->game_over_callback = f;
	}

	// Function to render the board to the screen (graphical window)
	void render(sf::RenderWindow *window) {
		// Fill screen with brown bg color
		window->clear(sf::Color(0xA67C5200));

		// Get window size and sprite sizes
		auto winSize = window->getSize();
		auto holeSize = holeSprite.getLocalBounds();
		auto moleSize = moleSprite.getLocalBounds();
		
		// Draw the holes or moles if there's a mole in the position
		for (int i = 0; i < x; i++) {
			for (int j = 0; j < y; j++) {
				// Set the hole and mole sprite positions based on the window size, sprite size and current position on board
				holeSprite.setPosition(sf::Vector2f(i * winSize.x/3 + 0.3*holeSize.width/2, j * winSize.y/3 + 0.3*holeSize.height/2));
				moleSprite.setPosition(sf::Vector2f(i * winSize.x/3 + 0.3*moleSize.width/2, j * winSize.y/3 + 0.3*moleSize.height/2));
				if (board[i][j]) {
					window->draw(moleSprite);
				} else {
					window->draw(holeSprite);
				}
			}
		}
	}

	// Stop function to stop all threads
	void stop() {
		for (int i = 0; i < x; i++)
			for (int j = 0; j < y; j++) {
				trt_mutex.lock();
				terminate_remove_thread[i][j] = true;
				trt_mutex.unlock();
			}
	}

private:
	int x, y;
	std::vector<std::vector<bool>> board;
	std::atomic_int n_whacked, n_missed, n_total;
	std::vector<std::chrono::duration<double>> whack_times;
	std::mt19937 rd;
	std::uniform_int_distribution<int> xdist, ydist;
	std::vector<std::vector<std::chrono::high_resolution_clock::time_point>> mole_start_times;
	std::function<void()> game_over_callback;
	std::vector<std::thread> m_threads;
	std::vector<std::vector<bool>> terminate_remove_thread;
	std::mutex trt_mutex = std::mutex();
	sf::Texture moleTex, holeTex;
	sf::Sprite moleSprite, holeSprite;

	// Function to remove mole at a position in 6 seconds
	void remove_mole(int x, int y) {
		// Get the start time
		auto start = std::chrono::high_resolution_clock::now();
		// Wait until 6 seconds have passed
		while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 7) {
			// Acquire the trt mutex
			trt_mutex.lock();
			// If the terminate_remove_thread for the location has been set, unset it, release mutex and return
			if (terminate_remove_thread[x][y]) {
				terminate_remove_thread[x][y] = false;
				trt_mutex.unlock();
				return;
			}
			// Else, unlock mutex and wait
			trt_mutex.unlock();
		}
		// If the time has passed, remove mole and increment the number of moles missed.
		board[x][y] = false;
		n_missed++;
	}
};

#endif // GAME_H