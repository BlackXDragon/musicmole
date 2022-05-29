/*
 * Create an SFML window and draw a circle.
 */

#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(400, 300));
	shape.setPointCount(20);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();
	}

	return 0;
}