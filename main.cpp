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

void OnDropDownSelect(sf::CircleShape* shape, sfg::ComboBox::Ptr combo) {
	auto index = combo->GetSelectedItem();
	std::cout << "Selected item: " << index << std::endl;
	switch(index) {
		case 0:
			shape->setFillColor(sf::Color::Red);
			break;
		case 1:
			shape->setFillColor(sf::Color::Green);
			break;
		case 2:
			shape->setFillColor(sf::Color::Blue);
			break;
		default:
			shape->setFillColor(sf::Color::White);
			break;
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

	auto dropdown = sfg::ComboBox::Create();
	dropdown->AppendItem("Red");
	dropdown->AppendItem("Green");
	dropdown->AppendItem("Blue");

	dropdown->GetSignal(sfg::ComboBox::OnSelect).Connect(
		std::bind(&OnDropDownSelect, &shape, dropdown)
	);

	auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
	box->Pack(button);
	box->Pack(dropdown);

	auto guiwindow = sfg::Window::Create();
	guiwindow->SetTitle("Hello World example");
	guiwindow->Add(box);

	sfg::Desktop desktop;
	desktop.Add(guiwindow);

	sf::Clock clock = sf::Clock();

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			desktop.HandleEvent(event);
			if (event.type == sf::Event::Closed)
				window.close();
		}

		desktop.Update(clock.getElapsedTime().asSeconds());

		window.clear();
		sfgui.Display(window);
		window.draw(shape);
		window.display();
	}

	return 0;
}