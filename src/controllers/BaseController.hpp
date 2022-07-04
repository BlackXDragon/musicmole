/*
 * Base Controller class
 */

#if !defined(BASECONTROLLER_H)
#define BASECONTROLLER_H

#include <functional>
#include <SFML/Graphics.hpp>

template<typename... Types>
class BaseController {
public:
	BaseController(std::function<void(Types... args)> callback) {
		this->callback = callback;
	}

	BaseController() = delete;

	virtual void run(sf::Event event) = 0;

protected:
	std::function<void(Types... args)> callback;
};

#endif // BASECONTROLLER_H