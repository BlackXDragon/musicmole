/*
 * Create an SFML window and draw a circle.
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>

void OnButtonClick(sfg::Button::Ptr button) {
	if (button->GetLabel() == "Hello") {
		button->SetLabel("World");
	} else {
		button->SetLabel("Hello");
	}
}

int main() {
	sfg::SFGUI sfgui;
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(sf::Vector2f(100, 100));
	shape.setPosition(sf::Vector2f(400, 300));
	shape.setPointCount(20);

	auto button = sfg::Button::Create("Hello");

	button->GetSignal(sfg::Button::OnLeftClick).Connect(
		std::bind(&OnButtonClick, button)
	);

	auto guiwindow = sfg::Window::Create();
	guiwindow->SetTitle("Hello World example");
	guiwindow->Add(button);

	sfg::Desktop desktop;
	desktop.Add(guiwindow);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			desktop.HandleEvent(event);
			if (event.type == sf::Event::Closed)
				window.close();
		}

		desktop.Update(1.0f);

		window.clear();
		sfgui.Display(window);
		window.draw(shape);
		window.display();
	}

	return 0;
}