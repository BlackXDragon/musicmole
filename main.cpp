/*
 * Filename: main.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Main file for the musicmole game.
 */

#include "src/game/game.hpp"
#include "src/controllers/NumericalController.hpp"
#include "src/controllers/GestureController.hpp"
#include "src/ticker/PeriodicTicker.hpp"
#include "src/ticker/MusicalTicker.hpp"
#include "src/UI/menu.hpp"
#include "src/UI/Roboto_Italic_Font.hpp"

// Define maximum FPS of the game
int MAX_FPS = 30;

// Boolean variable indicating when the menu should be closed and game started
bool closeMenu = false;

// Variables to store the controller and ticker params
std::variant<int, GestureControllerParams> controller_params;
std::variant<int, MusicalTickerParams> ticker_params;

// Callback function that will be called when start button is clicked on the menu
void startCallback(std::variant<int, GestureControllerParams> c, std::variant<int, MusicalTickerParams> t) {
	controller_params = c;
	ticker_params = t;
	closeMenu = true;
}

// Boolean variable indicating when the game has ended
bool end_game = false;

// Callback function that will be called when the game has ended
void game_over_callback() {
	end_game = true;
	std::cout << "Game over!" << std::endl;
}

int main() {
	// Initialise SFGUI and the renderwindow
	sfg::SFGUI sfgui;
	sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "Game with menu test", sf::Style::Fullscreen);

	// Get window size and set FPS limit
	auto windowSize = window.getSize();
	window.setFramerateLimit(MAX_FPS);

	// Create the SFGUI desktop
	sfg::Desktop desktop;

	// Create the menu window, update its size and set the start callback
	MenuWindow menu = MenuWindow(desktop, window);
	menu.updateSize(sf::Vector2f(window.getSize().x, window.getSize().y));
	menu.setStartCallback(startCallback);

	// Add the menu window to the SFGUI desktop
	desktop.Add(menu.getWindow());

	// Set the colour of separators in the SFGUI desktop
	desktop.GetEngine().SetProperty("Separator", "Color", sf::Color(0x754C24FF));

	// Initialise an SFML clock
	sf::Clock clock = sf::Clock();

	// Menu Window loop
	try {
		while (window.isOpen() && !closeMenu) {
			sf::Event event;
			while (window.pollEvent(event)) {
				// Handle GUI events for the menu window
				menu.update(event);
				desktop.HandleEvent(event);
				// Update the menu window size if the window size changes
				if (event.type == sf::Event::Resized)
					menu.updateSize(sf::Vector2f(event.size.width, event.size.height));
				if (event.type == sf::Event::Closed) {
					window.close();
				}
			}

			// Protect against rendering to a closed window
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

	// If the window is closed, exit
	if (!window.isOpen())
		exit(0);

	// Create the game variable and set the gameover callback
	Game game(3, 3);
	game.set_game_over_callback(&game_over_callback);

	// Load the font
	sf::Font font;
	font.loadFromMemory(Roboto_Italic_ttf, Roboto_Italic_ttf_len);

	// Create Text to display the number of moles hit, missed and total and the average response time
	sf::Text hit, missed, total, hitTime;
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
	hitTime.setFont(font);
	hitTime.setFillColor(sf::Color::White);
	hitTime.setCharacterSize(24);
	hitTime.setString("Average Response Time: -");

	// Set the positions of the texts
	auto textBounds = hit.getLocalBounds();
	hit.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, textBounds.height/2));
	textBounds = missed.getLocalBounds();
	missed.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 2*textBounds.height));
	textBounds = total.getLocalBounds();
	total.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 3.5*textBounds.height));
	textBounds = hitTime.getLocalBounds();
	hitTime.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 5*textBounds.height));
	
	// Create the controller and ticker based on the passed parameters
	std::unique_ptr<BaseController<int, int>> controller;
	serialib lserial, rserial;
	df_t lmodel, rmodel;
	std::unique_ptr<BaseTicker> ticker;

	if (controller_params.index() == 0) {
		// Numerical controller
		controller = std::make_unique<NumericalController>(std::bind(&Game::whack, &game, std::placeholders::_1, std::placeholders::_2));
	} else if (controller_params.index() == 1) {
		// Gesture Controller
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

		// Load gesture models
		loadGestureModel(lmodel, p.lmodelPath);
		loadGestureModel(rmodel, p.rmodelPath);
		controller = std::make_unique<GestureController>(lserial, lmodel, rserial, rmodel, std::bind(&Game::whack, &game, std::placeholders::_1, std::placeholders::_2));
	}

	if (ticker_params.index() == 0)
		// Periodic ticker
		ticker = std::make_unique<PeriodicTicker>(std::get<int>(ticker_params) * 1000, std::bind(&Game::make_mole, &game));
	else if (ticker_params.index() == 1) {
		// Musical Ticker
		MusicalTickerParams p = std::get<MusicalTickerParams>(ticker_params);
		ticker = std::make_unique<MusicalTicker>(p.filename, std::bind(&Game::make_mole, &game), p.analysisPeriod, p.lowFreq, p.highFreq, p.threshold, p.ignorePeriod);
	}

	// Clean up fftw since it is not needed anymore
	fftw_cleanup();

	// Start the ticker
	if (ticker_params.index() == 1)
		dynamic_cast<MusicalTicker&>(*ticker).start();
	else
		ticker->start();

	// Guard against exceptions
	try {
		while (window.isOpen() && !end_game && ticker->isRunning()) {
			sf::Event event;
			// If using a gesture controller, run it using an empty event (so that the controller updates even on framess that have no GUI event)
			if (controller_params.index() == 1)
				controller->run(sf::Event());
			while (window.pollEvent(event)) {
				// Pass GUI events to controller so that numerical controller works
				controller->run(event);
				if (event.type == sf::Event::KeyPressed)
					if (event.key.code == sf::Keyboard::Escape) {
						end_game = true;
						// Stop ticker when exiting
						if (ticker_params.index() == 1)
							dynamic_cast<MusicalTicker&>(*ticker).stop();
						else
							ticker->stop();
						// Stop game when exiting
						game.stop();
						break;
					}
				if (event.type == sf::Event::Closed)
					window.close();
			}

			// Update the values of moles hit, missed and total
			char h[5] = "", m[5] = "", t[5] = "", ht[15] = "";
			itoa(game.get_n_whacked(), h, 10);
			itoa(game.get_n_missed(), m, 10);
			itoa(game.get_n_total(), t, 10);
			sprintf(ht, "%0.2fms", game.get_avg_whack_time());
			
			// Create the strings
			std::string hitString = "Hit: ";
			hitString += h;
			std::string missedString = "Missed: ";
			missedString += m;
			std::string totalString = "Total: ";
			totalString += t;
			std::string hitTimeString = "Average Response Time: ";
			hitTimeString += ht;

			//Set the strings to the SFML Texts
			hit.setString(hitString);
			missed.setString(missedString);
			total.setString(totalString);
			hitTime.setString(hitTimeString);
			
			// Update the positions
			textBounds = hit.getLocalBounds();
			hit.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, textBounds.height/2));
			textBounds = missed.getLocalBounds();
			missed.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 2*textBounds.height));
			textBounds = total.getLocalBounds();
			total.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 3.5*textBounds.height));
			textBounds = hitTime.getLocalBounds();
			hitTime.setPosition(sf::Vector2f(windowSize.x - textBounds.width - 10, 5*textBounds.height));

			// Render the game and the texts
			window.clear();
			game.render(&window);
			window.draw(hit);
			window.draw(missed);
			window.draw(total);
			window.draw(hitTime);
			window.display();
		}
	} catch (std::exception e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(1);
	}

	// If the loop did not exit because of game over, manually stop the game
	if (!end_game) {
		end_game = true;
		game.stop();
	}

	// Guard against exceptions
	try {
		// Verify that the window is open
		if (window.isOpen()) {
			// Create game over text
			sf::Text gameOver;
			gameOver.setFont(font);
			gameOver.setFillColor(sf::Color::White);
			gameOver.setCharacterSize(36);
			gameOver.setString("Game Over!");
			textBounds = gameOver.getLocalBounds();
			gameOver.setPosition(sf::Vector2f(windowSize.x/2-(textBounds.width/2), windowSize.y/2-(textBounds.height/2)));

			// Start a clock to display the end game screen for 10 seconds
			auto start = std::chrono::high_resolution_clock::now();

			// GUI and time loop
			while ((std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < 10) && window.isOpen()) {
				sf::Event event;
				while (window.pollEvent(event)) {
					if (event.type == sf::Event::Closed)
						window.close();
				}

				// Draw the game over and score texts
				window.clear();
				window.draw(gameOver);
				window.draw(hit);
				window.draw(missed);
				window.draw(total);
				window.draw(hitTime);
				window.display();
			}
		}
	} catch (std::exception e) {
		std::cerr << "Error: " << e.what() << std::endl;
		exit(1);
	}

	// Print out the scores to the console
	std::cout << "Hit: " << game.get_n_whacked() << std::endl;
	std::cout << "Missed: " << game.get_n_missed() << std::endl;
	std::cout << "Total: " << game.get_n_total() << std::endl;
	std::cout << "Average response time: " << game.get_avg_whack_time() << std::endl;

	// Close serial devices if gesture controller was used
	if (controller_params.index() == 1) {
		lserial.closeDevice();
		rserial.closeDevice();
	}

	return 0;
}