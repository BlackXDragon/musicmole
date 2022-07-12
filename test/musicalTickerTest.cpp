/* 
 * Beat Detection test
 */

#include <SFML/Graphics.hpp>
#include "../src/ticker/MusicalTicker.hpp"
#include <iostream>

bool trigger = false;
std::chrono::high_resolution_clock::time_point trigger_time;

void callback() {
	trigger = true;
	trigger_time = std::chrono::high_resolution_clock::now();
}

int main() {
	std::unique_ptr<MusicalTicker> ticker;
	try {
		ticker = std::make_unique<MusicalTicker>(MUSIC_FILE, callback, std::chrono::milliseconds(10), 60, 500, 0.7, std::chrono::milliseconds(100));
	} catch(std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	std::cout << "Created ticker\n";

	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(400, 300));
	shape.setPointCount(50);

	ticker->start();
	while (window.isOpen() && ticker->isRunning()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		if (trigger && std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - trigger_time).count() <= 2000) {
			window.draw(shape);
		} else {
			trigger = false;
		}
		window.display();
	}

	if (window.isOpen())
		window.close();

	fftw_cleanup();
	return 0;
}