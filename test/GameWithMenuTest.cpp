#include "../src/game/game.hpp"
#include "../src/controllers/NumericalController.hpp"
#include "../src/controllers/GestureController.hpp"
#include "../src/ticker/PeriodicTicker.hpp"
#include "../src/ticker/MusicalTicker.hpp"
#include "../src/UI/menu.hpp"

int MAX_FPS = 30;

bool closeMenu = false;

std::variant<int, GestureControllerParams> controller_params;
std::variant<int, MusicalTickerParams> ticker_params;

void startCallback(std::variant<int, GestureControllerParams> c, std::variant<int, MusicalTickerParams> t) {
	controller_params = c;
	ticker_params = t;
	closeMenu = true;
}

bool end_game = false;

void game_over_callback() {
	end_game = true;
	std::cout << "Game over!" << std::endl;
}

int main() {
	sfg::SFGUI sfgui;
	sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "Game with menu test", sf::Style::Fullscreen);
	auto windowSize = window.getSize();
	window.setFramerateLimit(MAX_FPS);

	sfg::Desktop desktop;

	MenuWindow menu = MenuWindow(desktop, window);
	menu.updateSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	menu.setStartCallback(startCallback);

	desktop.Add(menu.getWindow());

	desktop.GetEngine().SetProperty("Separator", "Color", sf::Color(0x754C24FF));

	sf::Clock clock = sf::Clock();

	// Menu Window loop
	try {
		while (window.isOpen() && !closeMenu) {
			sf::Event event;
			while (window.pollEvent(event)) {
				menu.update(event);
				desktop.HandleEvent(event);
				if (event.type == sf::Event::Resized)
					menu.updateSize(sf::Vector2f(event.size.width, event.size.height));
				if (event.type == sf::Event::Closed) {
					window.close();
				}
			}

			if (window.isOpen()) {
				desktop.Update(clock.getElapsedTime().asSeconds());

				window.clear();
				menu.Draw(window);
				sfgui.Display(window);
				window.display();
			}
		}
	} catch (std::exception e) {
		std::cout << "Error: " << e.what() << std::endl;
	}

	if (!window.isOpen())
		exit(0);

	Game game(3, 3);
	game.set_game_over_callback(&game_over_callback);

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
	
	std::unique_ptr<BaseController<int, int>> controller;
	serialib lserial, rserial;
	df_t lmodel, rmodel;
	std::unique_ptr<BaseTicker> ticker;

	if (controller_params.index() == 0) {
		controller = std::make_unique<NumericalController>(std::bind(&Game::whack, &game, std::placeholders::_1, std::placeholders::_2));
	} else if (controller_params.index() == 1) {
		GestureControllerParams p = std::get<GestureControllerParams>(controller_params);
		// Try to open the serial ports.
		char err;
		try {
			err = lserial.openDevice(p.lCOMport.c_str(), 9600);
		} catch (std::exception& e) {
			std::cout << "Error opening left serial port " << p.lCOMport << ": " << e.what() << std::endl;
			return 1;
		}

		if (err != 1) {
			std::cout << "Error opening left serial port " << p.lCOMport << ": " << (int)err << std::endl;
			return err;
		}

		try {
			err = rserial.openDevice(p.rCOMport.c_str(), 9600);
		} catch (std::exception& e) {
			std::cout << "Error opening right serial port " << p.rCOMport << ": " << e.what() << std::endl;
			return 1;
		}

		if (err != 1) {
			std::cout << "Error opening right serial port " << p.rCOMport << ": " << (int)err << std::endl;
			return err;
		}

		loadGestureModel(lmodel, p.lmodelPath);
		loadGestureModel(rmodel, p.rmodelPath);
		controller = std::make_unique<GestureController>(lserial, lmodel, rserial, rmodel, std::bind(&Game::whack, &game, std::placeholders::_1, std::placeholders::_2));
	}

	if (ticker_params.index() == 0)
		ticker = std::make_unique<PeriodicTicker>(std::get<int>(ticker_params) * 1000, std::bind(&Game::make_mole, &game));
	else if (ticker_params.index() == 1) {
		MusicalTickerParams p = std::get<MusicalTickerParams>(ticker_params);
		ticker = std::make_unique<MusicalTicker>(p.filename, std::bind(&Game::make_mole, &game), p.analysisPeriod, p.lowFreq, p.highFreq, p.threshold, p.ignorePeriod);
	}

	fftw_cleanup();

	if (ticker_params.index() == 1)
		dynamic_cast<MusicalTicker&>(*ticker).start();
	else
		ticker->start();

	try {
		while (window.isOpen() && !end_game && ticker->isRunning()) {
			sf::Event event;
			if (controller_params.index() == 1)
				controller->run(sf::Event());
			while (window.pollEvent(event)) {
				controller->run(event);
				if (event.type == sf::Event::KeyPressed)
					if (event.key.code == sf::Keyboard::Escape) {
						end_game = true;
						if (ticker_params.index() == 1)
							dynamic_cast<MusicalTicker&>(*ticker).stop();
						else
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

	if (!end_game) {
		end_game = true;
		game.stop();
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

	if (controller_params.index() == 1) {
		lserial.closeDevice();
		rserial.closeDevice();
	}

	return 0;
}