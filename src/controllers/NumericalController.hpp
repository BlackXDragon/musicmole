/*
 * Filename: NumericalController.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class implementing the numerical (keyboard) controller.
 */

#if !defined(NumericalController_hpp)
#define NumericalController_hpp

#include <iostream>
#include "BaseController.hpp"

// NumericalController derived from BaseController with callback that accepts two ints that uses number keys 1-9 from the keyboard to map from (1, 1) to (3, 3)
class NumericalController : public BaseController<int, int> {
public:
	// NumericalController derived from BaseController with callback that accepts two ints that uses number keys 1-9 from the keyboard to map from (1, 1) to (3, 3)
	NumericalController(std::function<void(int, int)> callback) : BaseController<int, int>(callback) {}

	// Run function accepts an SFML event and calls the desired callback based on the number key pressed.
	void run(sf::Event event) {
		// Check if the event type is KeyReleased and if a key was previously pressed (to avoid multiple callbacks)
		if (event.type == sf::Event::KeyReleased && keyPressed) {
			// Set keyPressed to false to reset for next key
			keyPressed = false;
			// Based on the pressed key code, implement one of the callbacks.
			switch (event.key.code) {
				case sf::Keyboard::Num1:
				case sf::Keyboard::Numpad1:
					callback(1, 1);
					break;
				case sf::Keyboard::Num2:
				case sf::Keyboard::Numpad2:
					callback(2, 1);
					break;
				case sf::Keyboard::Num3:
				case sf::Keyboard::Numpad3:
					callback(3, 1);
					break;
				case sf::Keyboard::Num4:
				case sf::Keyboard::Numpad4:
					callback(1, 2);
					break;
				case sf::Keyboard::Num5:
				case sf::Keyboard::Numpad5:
					callback(2, 2);
					break;
				case sf::Keyboard::Num6:
				case sf::Keyboard::Numpad6:
					callback(3, 2);
					break;
				case sf::Keyboard::Num7:
				case sf::Keyboard::Numpad7:
					callback(1, 3);
					break;
				case sf::Keyboard::Num8:
				case sf::Keyboard::Numpad8:
					callback(2, 3);
					break;
				case sf::Keyboard::Num9:
				case sf::Keyboard::Numpad9:
					callback(3, 3);
					break;
			}
		} else if (event.type == sf::Event::KeyPressed) {
			// If there was a key pressed event, set keyPressed to true and save the code of the pressed key.
			keyPressed = true;
			pressed = event.key.code;
		}
	}

private:
	bool keyPressed = false;
	sf::Keyboard::Key pressed;
};

#endif // NumericalController_hpp