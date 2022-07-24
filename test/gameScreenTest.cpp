#include "../src/game/game.hpp"

bool end_game = false;

void game_over_callback() {
	end_game = true;
	std::cout << "Game over!" << std::endl;
}

int main() {
	sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "SFML works!", sf::Style::Fullscreen);
	auto windowSize = window.getSize();
	
	Game game(3, 3);
	game.set_game_over_callback(&game_over_callback);

	game.make_mole();

	try {
		while (window.isOpen() && !end_game) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::KeyPressed)
					if (event.key.code == sf::Keyboard::Escape) {
						end_game = true;
						game.stop();
						break;
					}
				if (event.type == sf::Event::Closed)
					window.close();
			}

			window.clear();
			game.render(&window);
			window.display();
		}
	} catch (std::exception e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(1);
	}

	return 0;
}