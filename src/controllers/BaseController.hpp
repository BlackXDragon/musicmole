/*
 * Filename: BaseController.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class implementing the core features of a controller.
 */

#if !defined(BASECONTROLLER_H)
#define BASECONTROLLER_H

#include <functional>
#include <SFML/Graphics.hpp>

// Templated BaseController class that allows using callback with any types and number of arguments.
template<typename... Types>
class BaseController {
public:
	// Constructor accepts the required callback function of arbitrary number and type of arguments.
	BaseController(std::function<void(Types... args)> callback) {
		this->callback = callback;
	}

	// Delete the default constructor since a callback function is required.
	BaseController() = delete;

	// Virtual 'run' function that accepts an SFML event as parameter to be implemented by derived classes.
	virtual void run(sf::Event event) = 0;

protected:
	std::function<void(Types... args)> callback;
};

#endif // BASECONTROLLER_H