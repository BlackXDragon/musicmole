/*
 * Class for the game logic.
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

class Game {
public:
	Game() : Game(3, 3) {}

	Game(int x = 3, int y = 3) {
		this->x = x;
		this->y = y;
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
		this->n_whacked = 0;
		this->n_missed = 0;
		this->n_total = 0;
		this->whack_times = std::vector<std::chrono::duration<double>>();
		this->rd = std::mt19937(std::random_device()());
		this->xdist = std::uniform_int_distribution<int>(0, x - 1);
		this->ydist = std::uniform_int_distribution<int>(0, y - 1);
	}

	void reset() {
		this->n_total = 0;
		this->n_whacked = 0;
		this->n_missed = 0;
		this->whack_times.clear();
	}

	void make_mole() {
		int x = xdist(rd);
		int y = ydist(rd);
		while (board[x][y]) {
			x = xdist(rd);
			y = ydist(rd);
		}
		board[x][y] = true;
		bool board_full = true;
		for (int i = 0; i < this->x; i++)
			for (int j = 0; j < this->y; j++)
				if (!board[i][j])
					board_full = false;
		if (board_full) {
			this->reset();
			this->game_over_callback();
		}
		this->mole_start_times[x][y] = std::chrono::high_resolution_clock::now();
		m_threads.push_back(std::thread(&Game::remove_mole, this, x, y));
	}

	void whack(int x, int y) {
		if (board[x][y]) {
			board[x][y] = false;
			n_whacked++;
			whack_times.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - mole_start_times[x][y]));
			n_total++;
			terminate_remove_thread[x][y] = true;
		}
	}

	void print_board() {
		std::cout << "+---+---+---+\n";
		for (int i = 0; i < this->x; i++) {
			for (int j = 0; j < this->y; j++) {
				std::cout << "| " << (board[i][j] ? "#" : " ") << " ";
			}
			std::cout << "|\n+---+---+---+\n";
		}
	}

	int get_n_whacked() {
		return n_whacked;
	}

	int get_n_missed() {
		return n_missed;
	}

	int get_n_total() {
		return n_total;
	}

	std::vector<std::chrono::duration<double>> get_whack_times() {
		return whack_times;
	}

	double get_avg_whack_time() {
		double sum = 0;
		for (auto time : whack_times) {
			sum += std::chrono::duration_cast<std::chrono::milliseconds>(time).count();
		}
		return sum / whack_times.size();
	}

	double get_score() {
		return (double)n_whacked / n_total;
	}
	
	std::vector<std::vector<bool>> get_board() {
		return board;
	}

	int get_x() {
		return x;
	}

	int get_y() {
		return y;
	}

	void set_game_over_callback(std::function<void()> f) {
		this->game_over_callback = f;
	}

private:
	int x, y;
	std::vector<std::vector<bool>> board;
	int n_whacked, n_missed, n_total;
	std::vector<std::chrono::duration<double>> whack_times;
	std::mt19937 rd;
	std::uniform_int_distribution<int> xdist, ydist;
	std::vector<std::vector<std::chrono::high_resolution_clock::time_point>> mole_start_times;
	std::function<void()> game_over_callback;
	std::vector<std::thread> m_threads;
	std::vector<std::vector<bool>> terminate_remove_thread;
	
	void remove_mole(int x, int y) {
		auto start = std::chrono::high_resolution_clock::now();
		while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 7) {
			if (terminate_remove_thread[x][y]) {
				terminate_remove_thread[x][y] = false;
				return;
			}
		}
		board[x][y] = false;
		n_missed++;
		n_total++;
	}
};

#endif // GAME_H