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

void startCallback(int c, std::string c1, std::string c2, std::variant<int, MusicalTickerParams> t) {
	if (t.index() == 0)
		std::cout << "Got: " << c << ", " << c1 << ", " << c2 << ", Periodic Ticker with period: " << std::get<int>(t) << std::endl;
	else
		std::cout << "Got: " << c << ", " << c1 << ", " << c2
			<< "\nMusical Ticker: "
			<< "\nFilename: " << std::get<MusicalTickerParams>(t).filename
			<< "\nFreq range: " << std::get<MusicalTickerParams>(t).lowFreq << " - " << std::get<MusicalTickerParams>(t).highFreq
			<< "\nThreshold: " << std::get<MusicalTickerParams>(t).threshold
			<< "\nAnalysis period: " << std::get<MusicalTickerParams>(t).analysisPeriod.count() / 1000 << " ms"
			<< "\nIgnore period: " << std::get<MusicalTickerParams>(t).ignorePeriod.count() / 1000 << " ms"
			<< std::endl;
	close = true;
}

int main() {
	sfg::SFGUI sfgui;
	sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "Menu Test", sf::Style::Fullscreen);
	window.setFramerateLimit(MAX_FPS);
	
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(-100, -100));
	shape.setPointCount(20);

	sfg::Desktop desktop;
	
	MenuWindow menu = MenuWindow(desktop, window);
	menu.updateSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	menu.setStartCallback(startCallback);

	desktop.Add(menu.getWindow());

	desktop.GetEngine().SetProperty("Separator", "Color", sf::Color(0x754C24FF));

	sf::Clock clock = sf::Clock();

	while (window.isOpen() && !close) {
		sf::Event event;
		while (window.pollEvent(event)) {
			desktop.HandleEvent(event);
			menu.update(event);
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