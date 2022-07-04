/*
 * Test for the Numerical Controller
 */

#include "../src/controllers/NumericalController.hpp"
#include <SFML/Graphics.hpp>

std::string s = "Press a number between 1-9";

void callback(int x, int y) {
	s = "You pressed: ";
	char c[1];
	itoa(x + (y - 1) * 3, c, 10);
	s += c;

	std::cout << s << std::endl;
}

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 600), "Numerical Controller Test");
	sf::Font font;
	font.loadFromFile("Roboto-Italic.ttf");
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);
	text.setCharacterSize(24);

	sf::Vector2u windowSize = window.getSize();

	NumericalController controller = NumericalController(callback);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			controller.run(event);
			if (event.type == sf::Event::Closed)
				window.close();
		}

		text.setString(s);
		auto textBounds = text.getLocalBounds();
		text.setPosition(sf::Vector2f(windowSize.x/2-(textBounds.width/2), windowSize.y/2-(textBounds.height/2)));

		window.clear();
		window.draw(text);
		window.display();
	}

	return 0;
}