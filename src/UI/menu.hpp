#if !defined(MENU_HPP)
#define MENU_HPP

#include "../serial/availableSerial.hpp"
#include "../ticker/PeriodicTicker.hpp"
#include "../controllers/NumericalController.hpp"
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <SFML/Graphics.hpp>

class MenuWindow {
public:
	MenuWindow(sfg::Desktop& desktop) {
		background = sf::RectangleShape(sf::Vector2f(600, 600));
		background.setFillColor(sf::Color(0xA67C52FF));
		
		box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		box->SetRequisition(sf::Vector2f(600, 600));
		box->SetPosition(sf::Vector2f(200, 200));

		auto label = sfg::Label::Create("Music Mole Settings");
		desktop.GetEngine().SetProperty("Label", "FontSize", 24);
		box->Pack(label);

		auto line = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		box->Pack(line);
		
		auto controllerLabel = sfg::Label::Create("Controller Settings");
		box->Pack(controllerLabel);

		auto controllerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		
		auto controllerModeLabel = sfg::Label::Create("Contoller mode:");
		controllerBox->Pack(controllerModeLabel);
		
		controllerDropdown = sfg::ComboBox::Create();
		controllerDropdown->AppendItem("Numerical");
		controllerDropdown->AppendItem("Glove");

		controllerDropdown->GetSignal(sfg::ComboBox::OnSelect).Connect(
			std::bind(&MenuWindow::onControllerDropdownChange, this)
		);

		controllerBox->Pack(controllerDropdown);

		box->Pack(controllerBox);

		glovesBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		
		auto glovesBoxLeft = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		auto glovesBoxRight = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);

		auto glovesLabelLeft = sfg::Label::Create("Left Glove");
		auto glovesLabelRight = sfg::Label::Create("Right Glove");

		glovesBoxLeft->Pack(glovesLabelLeft);
		glovesBoxRight->Pack(glovesLabelRight);

		auto serialBoxLeft = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		auto serialBoxRight = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto serialLabelLeft = sfg::Label::Create("COM Port");
		auto serialLabelRight = sfg::Label::Create("COM Port");

		serialBoxLeft->Pack(serialLabelLeft);
		serialBoxRight->Pack(serialLabelRight);

		serialComboLeft = sfg::ComboBox::Create();
		serialComboRight = sfg::ComboBox::Create();

		serialBoxLeft->Pack(serialComboLeft);
		serialBoxRight->Pack(serialComboRight);

		auto serialButtonLeft = sfg::Button::Create("Refresh");
		auto serialButtonRight = sfg::Button::Create("Refresh");

		serialButtonLeft->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshCombo, this, serialComboLeft)
		);

		serialButtonRight->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshCombo, this, serialComboRight)
		);

		serialBoxLeft->Pack(serialButtonLeft);
		serialBoxRight->Pack(serialButtonRight);

		glovesBoxLeft->Pack(serialBoxLeft);
		glovesBoxRight->Pack(serialBoxRight);
		
		auto modelBoxLeft = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		auto modelBoxRight = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto modelLabelLeft = sfg::Label::Create("Gesture Model");
		auto modelLabelRight = sfg::Label::Create("Gesture Model");

		modelBoxLeft->Pack(modelLabelLeft);
		modelBoxRight->Pack(modelLabelRight);

		auto modelComboLeft = sfg::ComboBox::Create();
		auto modelComboRight = sfg::ComboBox::Create();

		modelBoxLeft->Pack(modelComboLeft);
		modelBoxRight->Pack(modelComboRight);

		auto modelButtonLeft = sfg::Button::Create("Delete");
		auto modelButtonRight = sfg::Button::Create("Delete");

		modelBoxLeft->Pack(modelButtonLeft);
		modelBoxRight->Pack(modelButtonRight);

		glovesBoxLeft->Pack(modelBoxLeft);
		glovesBoxRight->Pack(modelBoxRight);

		auto modelNewButtonLeft = sfg::Button::Create("Train New Model");
		auto modelNewButtonRight = sfg::Button::Create("Train New Model");

		glovesBoxLeft->Pack(modelNewButtonLeft);
		glovesBoxRight->Pack(modelNewButtonRight);

		glovesBox->Pack(glovesBoxLeft);
		glovesBox->Pack(glovesBoxRight);

		box->Pack(glovesBox);

		// Ticker Settings

		line = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		box->Pack(line);

		auto tickerLabel = sfg::Label::Create("Ticker Settings");
		box->Pack(tickerLabel);

		auto tickerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		
		auto tickerModeLabel = sfg::Label::Create("Ticker mode:");
		tickerBox->Pack(tickerModeLabel);
		
		tickerDropdown = sfg::ComboBox::Create();
		tickerDropdown->AppendItem("Periodic");
		tickerDropdown->AppendItem("Musical");

		tickerDropdown->GetSignal(sfg::ComboBox::OnSelect).Connect(
			std::bind(&MenuWindow::onTickerDropdownChange, this)
		);

		tickerBox->Pack(tickerDropdown);

		box->Pack(tickerBox);

		periodicTickerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		musicalTickerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerPeriodLabel = sfg::Label::Create("Ticker Period (s):");
		periodicTickerBox->Pack(tickerPeriodLabel);
		
		tickerPeriodSpinButton = sfg::SpinButton::Create(1, 15, 1);
		periodicTickerBox->Pack(tickerPeriodSpinButton);

		auto tickerMusicLabel = sfg::Label::Create("Ticker Music (not yet implemented)");
		musicalTickerBox->Pack(tickerMusicLabel);

		box->Pack(periodicTickerBox);
		box->Pack(musicalTickerBox);

		auto startButton = sfg::Button::Create("Start!");
		box->Pack(startButton);

		startButton->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::startGame, this)
		);

		// Reset
		controllerDropdown->SelectItem(0);
		onControllerDropdownChange();
		tickerDropdown->SelectItem(0);
		onTickerDropdownChange();
	}

	~MenuWindow() {}

	void refreshCombo(sfg::ComboBox::Ptr combo) {
		while (combo->GetItemCount() > 0) {
			combo->RemoveItem(0);
		}
		auto availableSerialPorts = SelectComPort();
		for (auto& port : availableSerialPorts) {
			combo->AppendItem(port);
		}
	}
	
	void updateSize(sf::Vector2f size) {
		float x = size.x/10;
		float y = size.y/10;
		float w = size.x - x*2;
		float h = size.y - y*2;
		box->SetPosition(sf::Vector2f(x, y));
		box->SetRequisition(sf::Vector2f(w, h));

		background.setPosition(sf::Vector2f(x-8, y-8));
		background.setSize(sf::Vector2f(w+16, h+16));
	}

	void Draw(sf::RenderWindow& window) {
		window.draw(background);
	}

	sfg::Widget::Ptr getWindow() {
		return box;
	}

	void onControllerDropdownChange() {
		glovesBox->Show(true);
		if (this->controllerDropdown->GetSelectedItem() == 0)
			glovesBox->Show(false);
	}

	void onTickerDropdownChange() {
		switch (this->tickerDropdown->GetSelectedItem()) {
			case 0:
				periodicTickerBox->Show(true);
				musicalTickerBox->Show(false);
				break;
			case 1:
				periodicTickerBox->Show(false);
				musicalTickerBox->Show(true);
				break;
		}
	}

	// To be defined
	void startGame() {
		int c = this->controllerDropdown->GetSelectedItem();
		int t = this->tickerDropdown->GetSelectedItem();
		if (c == 0 && t == 0) {
			char v[5] = "";
			itoa(tickerPeriodSpinButton->GetValue(), v, 10);
			startCallback(0, "", "", 0, std::string(v));
		} // Rest to be implemented
	}

	void setStartCallback(std::function<void(int, std::string, std::string, int, std::string)> startCallback) {
		this->startCallback = startCallback;
	}

private:
	sfg::Box::Ptr box, glovesBox, periodicTickerBox, musicalTickerBox;
	sfg::ComboBox::Ptr serialComboLeft, serialComboRight, controllerDropdown, tickerDropdown;
	sf::RectangleShape background;
	sfg::SpinButton::Ptr tickerPeriodSpinButton;
	std::function<void(int, std::string, std::string, int, std::string)> startCallback;
};

#endif // MENU_HPP