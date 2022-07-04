/*
 * Create an SFML window and draw a circle.
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "../src/UI/menu.hpp"

int MAX_FPS = 30;

bool close = false;

void startCallback(int c, std::string c1, std::string c2, int t, std::string t1) {
	std::cout << "Got: " << c << ", " << c1 << ", " << c2 << ", " << t << ", " << t1 << std::endl;
	close = true;
}

int main() {
	sfg::SFGUI sfgui;
	sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "SFML works!", sf::Style::Fullscreen);
	window.setFramerateLimit(MAX_FPS);
	
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(-100, -100));
	shape.setPointCount(20);

	sfg::Desktop desktop;
	
	MenuWindow menu = MenuWindow(desktop);
	menu.updateSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	menu.setStartCallback(startCallback);

	desktop.Add(menu.getWindow());

	desktop.GetEngine().SetProperty("Separator", "Color", sf::Color(0x754C24FF));

	sf::Clock clock = sf::Clock();

	while (window.isOpen() && !close) {
		sf::Event event;
		while (window.pollEvent(event)) {
			desktop.HandleEvent(event);
			if (event.type == sf::Event::Resized)
				menu.updateSize(sf::Vector2f(event.size.width, event.size.height));
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (window.isOpen()) {
			desktop.Update(clock.getElapsedTime().asSeconds());

			window.clear();
			menu.Draw(window);
			sfgui.Display(window);
			window.draw(shape);
			window.display();
		}
	}

	if (window.isOpen())
		window.close();

	return 0;
}