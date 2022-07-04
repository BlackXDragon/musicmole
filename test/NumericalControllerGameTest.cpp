#include "../src/game/game.hpp"
#include "../src/controllers/NumericalController.hpp"
#include "../src/ticker/PeriodicTicker.hpp"

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
	std::unique_ptr<PeriodicTicker> ticker = std::make_unique<PeriodicTicker>(3000, std::bind(&Game::make_mole, &game));

	ticker->start();

	sf::Font font;
	font.loadFromFile("Roboto-Italic.ttf");

	sf::Text hit, missed, total;
	hit.setFont(font);
	hit.setFillColor(sf::Color::White);
	hit.setCharacterSize(24);
	hit.setString("Hit: 0");
	missed.setFont(font);
	missed.setFillColor(sf::Color::White);
	missed.setCharacterSize(24);
	missed.setString("Missed: 0");
	total.setFont(font);
	total.setFillColor(sf::Color::White);
	total.setCharacterSize(24);
	total.setString("Total: 0");

	auto textBounds = hit.getLocalBounds();
	hit.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, textBounds.height/2));
	textBounds = missed.getLocalBounds();
	missed.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 2*textBounds.height));
	textBounds = total.getLocalBounds();
	total.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 3.5*textBounds.height));

	NumericalController controller = NumericalController(std::bind(&Game::whack, &game, std::placeholders::_1, std::placeholders::_2));

	ticker->start();

	try {
		while (window.isOpen() && !end_game) {
			sf::Event event;
			while (window.pollEvent(event)) {
				controller.run(event);
				if (event.type == sf::Event::KeyPressed)
					if (event.key.code == sf::Keyboard::Escape) {
						end_game = true;
						ticker->stop();
						game.stop();
						break;
					}
				if (event.type == sf::Event::Closed)
					window.close();
			}

			char h[5] = "", m[5] = "", t[5] = "";
			itoa(game.get_n_whacked(), h, 10);
			itoa(game.get_n_missed(), m, 10);
			itoa(game.get_n_total(), t, 10);
			
			std::string hitString = "Hit: ";
			hitString += h;
			std::string missedString = "Missed: ";
			missedString += m;
			std::string totalString = "Total: ";
			totalString += t;

			hit.setString(hitString);
			missed.setString(missedString);
			total.setString(totalString);textBounds = hit.getLocalBounds();
			
			hit.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, textBounds.height/2));
			textBounds = missed.getLocalBounds();
			missed.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 2*textBounds.height));
			textBounds = total.getLocalBounds();
			total.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 3.5*textBounds.height));

			window.clear();
			game.render(&window);
			window.draw(hit);
			window.draw(missed);
			window.draw(total);
			window.display();
		}
	} catch (std::exception e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(1);
	}

	try {
		if (window.isOpen()) {
			sf::Text gameOver;
			gameOver.setFont(font);
			gameOver.setFillColor(sf::Color::White);
			gameOver.setCharacterSize(36);
			gameOver.setString("Game Over!");
			textBounds = gameOver.getLocalBounds();
			gameOver.setPosition(sf::Vector2f(windowSize.x/2-(textBounds.width/2), windowSize.y/2-(textBounds.height/2)));

			auto start = std::chrono::high_resolution_clock::now();

			while ((std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 10) && window.isOpen()) {
				sf::Event event;
				while (window.pollEvent(event)) {
					if (event.type == sf::Event::Closed)
						window.close();
				}

				window.clear();
				window.draw(gameOver);
				window.draw(hit);
				window.draw(missed);
				window.draw(total);
				window.display();
			}
		}
	} catch (std::exception e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(1);
	}

	std::cout << "Hit: " << game.get_n_whacked() << std::endl;
	std::cout << "Missed: " << game.get_n_missed() << std::endl;
	std::cout << "Total: " << game.get_n_total() << std::endl;

	return 0;
}