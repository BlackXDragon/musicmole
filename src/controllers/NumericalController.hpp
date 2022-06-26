#if !defined(NumericalController_hpp)
#define NumericalController_hpp

#include <iostream>
#include "BaseController.hpp"

class NumericalController : public BaseController<int, int> {
public:
	NumericalController(std::function<void(int, int)> callback) : BaseController<int, int>(callback) {}

	void run(sf::Event event) {
		if (event.type == sf::Event::KeyReleased && keyPressed) {
			keyPressed = false;
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
			keyPressed = true;
			pressed = event.key.code;
		}
	}

private:
	bool keyPressed = false;
	sf::Keyboard::Key pressed;
};

#endif // NumericalController_hpp