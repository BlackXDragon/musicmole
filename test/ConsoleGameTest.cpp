/*
 * Test the Game Logic and Periodic Ticker on the Console.
 */

#include "../src/game/game.hpp"
#include "../src/ticker/ticker"

bool end_game = false;

void input_thread(Game *game) {
	std::string input;
	while (!end_game) {
		std::cin >> input;
		if (input == "q") {
			end_game = true;
			std::cout << "Quit!" << std::endl;
			break;
		} else if (input == "r") {
			game->reset();
			std::cout << "\033[K\033[F";
		} else {
			std::cout << "\033[K\033[F";
			int n = std::stoi(input) - 1;
			if (!(n < game->get_x() * game->get_y()))
				continue;
			game->whack(n / game->get_x(), n % game->get_y());
		}
	}
}

void game_over_callback() {
	end_game = true;
	std::cout << "Game over!" << std::endl;
}

int main() {
	Game game(3, 3);
	game.set_game_over_callback(&game_over_callback);
	std::unique_ptr<PeriodicTicker> ticker = std::make_unique<PeriodicTicker>(5000, std::bind(&Game::make_mole, &game));

	// Start the input thread.
	std::thread input_thread(&input_thread, &game);

	ticker->start();

	// Game Loop
	while (!end_game) {
		game.print_board();
		std::cout << "Whacked: " << game.get_n_whacked() << std::endl;
		std::cout << "Missed: " << game.get_n_missed() << std::endl;
		std::cout << "Total: " << game.get_n_total() << std::endl;
		std::cout << "Average Whack Time: " << game.get_avg_whack_time() << " ms" << std::endl;
		for (int i = 0; i < 11; i++) {
			std::cout << "\033[K\033[F";
		}
	}
	std::cout << "Whacked: " << game.get_n_whacked() << std::endl;
	std::cout << "Missed: " << game.get_n_missed() << std::endl;
	std::cout << "Total: " << game.get_n_total() << std::endl;
	std::cout << "Average Whack Time: " << game.get_avg_whack_time() << " ms" << std::endl;

	ticker->stop();

	return 0;
}