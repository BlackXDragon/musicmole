/* 
 * Beat Detection test
 */

#include <SFML/Graphics.hpp>
#include "../src/beat_detection/beat_detection.hpp"
#include <iostream>

int main() {
	sf::SoundBuffer buf;
	try {
		buf.loadFromFile(MUSIC_FILE);
	} catch(std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	std::cout << "Loaded sound buffer\n";

	std::vector<std::chrono::microseconds> beats;
	try {
		beats = detectBeatTimes(buf, std::chrono::milliseconds(10), 60, 500, 0.7, std::chrono::milliseconds(100));
	} catch (std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
		fftw_cleanup();
		return 1;
	}

	std::cout << "Found " << beats.size() << " beats.\n";

	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(400, 300));
	shape.setPointCount(50);

	sf::Music music;
	music.openFromFile(MUSIC_FILE);
	int duration = music.getDuration().asMicroseconds();

	music.play();
	int offset = music.getPlayingOffset().asMicroseconds();
	while (window.isOpen() && offset <= duration) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		offset = music.getPlayingOffset().asMicroseconds();
		for (auto &i: beats) {
			if (offset >= i.count() && offset <= (i.count() + 2000))
				window.draw(shape);
		}
		window.display();
	}

	if (window.isOpen())
		window.close();

	fftw_cleanup();
	return 0;
}