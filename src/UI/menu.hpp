#if !defined(MENU_HPP)
#define MENU_HPP

#include <variant>
#include <filesystem>
#include <atomic>
#include "../serial/availableSerial.hpp"
#include "../ticker/PeriodicTicker.hpp"
#include "../ticker/MusicalTicker.hpp"
#include "../controllers/NumericalController.hpp"
#include "modelTrainPopup.hpp"
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <SFML/Graphics.hpp>
#include <serialib.h>

namespace fs = std::filesystem;

#ifdef _WIN32
	std::string homepath = std::string(getenv("HOMEDRIVE")) + std::string(getenv("HOMEPATH"));
	std::string musicpath = homepath + "\\Documents\\musicmole\\Music";
	std::string lmodelpath = homepath + "\\Documents\\musicmole\\Models\\Left\\";
	std::string rmodelpath = homepath + "\\Documents\\musicmole\\Models\\Right\\";
#endif
#ifdef linux
	std::string homepath = std::string(getenv("HOME"));
	std::string musicpath = homepath + "/musicmole/Music";
	std::string lmodelpath = homepath + "/musicmole/Models/Left/";
	std::string rmodelpath = homepath + "/musicmole/Models/Right/";
#endif

typedef struct {
	std::string filename;
	double lowFreq, highFreq, threshold;
	std::chrono::microseconds analysisPeriod, ignorePeriod;
} MusicalTickerParams;

class MenuWindow {
public:
	MenuWindow(sfg::Desktop& desktop, sf::RenderWindow& window) {
		this->desktop = &desktop;
		this->window = &window;
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

		modelNewButtonLeft->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::newModelButtonCallback, this, true)
		);

		modelNewButtonRight->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::newModelButtonCallback, this, false)
		);

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
		musicalTickerBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);

		auto tickerPeriodLabel = sfg::Label::Create("Ticker Period (s):");
		periodicTickerBox->Pack(tickerPeriodLabel);
		
		tickerPeriodSpinButton = sfg::SpinButton::Create(1, 15, 1);
		periodicTickerBox->Pack(tickerPeriodSpinButton);

		auto tickerMusicFileBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicFileLabel = sfg::Label::Create("Music:");
		tickerMusicFileCombo = sfg::ComboBox::Create();
		auto tickerMusicFileButton = sfg::Button::Create("Refresh");

		tickerMusicFileButton->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshMusic, this, tickerMusicFileCombo)
		);

		tickerMusicFileBox->Pack(tickerMusicFileLabel);
		tickerMusicFileBox->Pack(tickerMusicFileCombo);
		tickerMusicFileBox->Pack(tickerMusicFileButton);
		musicalTickerBox->Pack(tickerMusicFileBox);

		auto tickerMusicFreqBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicFreqLabel = sfg::Label::Create("Beat frequency range:");
		tickerLowFreqSpinButton = sfg::SpinButton::Create(20, 2000, 1);
		auto tickerMusicFreqHyphen = sfg::Label::Create("-");
		tickerHighFreqSpinButton = sfg::SpinButton::Create(100, 20000, 1);

		tickerMusicFreqBox->Pack(tickerMusicFreqLabel);
		tickerMusicFreqBox->Pack(tickerLowFreqSpinButton);
		tickerMusicFreqBox->Pack(tickerMusicFreqHyphen);
		tickerMusicFreqBox->Pack(tickerHighFreqSpinButton);
		musicalTickerBox->Pack(tickerMusicFreqBox);

		auto tickerMusicThresholdBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicThresholdLabel = sfg::Label::Create("Beat threshold:");
		tickerThresholdScale = sfg::Scale::Create(0, 1, 0.025);
		tickerMusicThresholdValue = sfg::Label::Create("0");

		box->GetSignal(sfg::Box::OnMouseMove).Connect(
			std::bind(&MenuWindow::updateThresholdValue, this)
		);

		tickerMusicThresholdBox->Pack(tickerMusicThresholdLabel);
		tickerMusicThresholdBox->Pack(tickerThresholdScale);
		tickerMusicThresholdBox->Pack(tickerMusicThresholdValue);
		musicalTickerBox->Pack(tickerMusicThresholdBox);

		auto tickerMusicAnalysisBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicAnalysisLabel = sfg::Label::Create("FFT Analysis period:");
		tickerAnalysisSpinButton = sfg::SpinButton::Create(1, 1000, 1);
		auto tickerMusicAnalysisMS = sfg::Label::Create("ms");

		tickerMusicAnalysisBox->Pack(tickerMusicAnalysisLabel);
		tickerMusicAnalysisBox->Pack(tickerAnalysisSpinButton);
		tickerMusicAnalysisBox->Pack(tickerMusicAnalysisMS);
		musicalTickerBox->Pack(tickerMusicAnalysisBox);

		auto tickerMusicIgnoreBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicIgnoreLabel = sfg::Label::Create("FFT Ignore period:");
		tickerIgnoreSpinButton = sfg::SpinButton::Create(1, 1000, 1);
		auto tickerMusicIgnoreMS = sfg::Label::Create("ms");

		tickerMusicIgnoreBox->Pack(tickerMusicIgnoreLabel);
		tickerMusicIgnoreBox->Pack(tickerIgnoreSpinButton);
		tickerMusicIgnoreBox->Pack(tickerMusicIgnoreMS);
		musicalTickerBox->Pack(tickerMusicIgnoreBox);

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
		refreshMusic(tickerMusicFileCombo);
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

	void refreshMusic(sfg::ComboBox::Ptr combo) {
		while (combo->GetItemCount() > 0) {
			combo->RemoveItem(0);
		}
		if (!fs::is_directory(fs::path(musicpath)))
			fs::create_directories(fs::path(musicpath));
		for (auto& file : fs::directory_iterator(musicpath)) {
			combo->AppendItem(std::string(file.path().filename().u8string()));
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
		if (modelTraining) {
			box->Show(false);
			modelTrainer->render(window);
		} else {
			box->Show(true);
			window.draw(background);
		}
	}

	void update(sf::Event& event) {
		if (modelTraining) {
			modelTrainer->update(event);
		}
	}

	void newModelButtonCallback(bool left = true) {
		std::string port;
		if (left)
			port = this->serialComboLeft->GetSelectedText();
		else
			port = this->serialComboRight->GetSelectedText();
		if (port == "") return;
		
		this->serial = new serialib();
		char err = this->serial->openDevice(port.c_str(), 9600);
		if (err != 1) throw std::exception("Error opening COM Port");

		if (left)
			this->modelTrainer = new NewModelTrainer(*serial, lmodelpath, sf::Vector2f(window->getSize().x, window->getSize().y), desktop, std::bind(&MenuWindow::modelTrainedCallback, this));
		else
			this->modelTrainer = new NewModelTrainer(*serial, rmodelpath, sf::Vector2f(window->getSize().x, window->getSize().y), desktop, std::bind(&MenuWindow::modelTrainedCallback, this));
		this->modelTraining = true;
	}

	void modelTrainedCallback() {
		modelTraining = false;
		serial->closeDevice();
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

	void startGame() {
		int c = this->controllerDropdown->GetSelectedItem();
		int t = this->tickerDropdown->GetSelectedItem();
		if (c == 0 && t == 0) {
			int v = tickerPeriodSpinButton->GetValue();
			std::variant<int, MusicalTickerParams> tickerParam = v;
			std::visit([this](auto &p){this->startCallback(0, "", "", p);}, tickerParam);
		} else if (c == 0 && t == 1) {
			MusicalTickerParams p = MusicalTickerParams{};
			#ifdef _WIN32
				p.filename = musicpath + "\\" + tickerMusicFileCombo->GetSelectedText();
			#endif
			#ifdef linux
				p.filename = musicpath + "/" + tickerMusicFileCombo->GetSelectedText();
			#endif
			p.lowFreq = tickerLowFreqSpinButton->GetValue();
			p.highFreq = tickerHighFreqSpinButton->GetValue();
			p.threshold = tickerThresholdScale->GetValue();
			p.analysisPeriod = std::chrono::milliseconds((int)tickerAnalysisSpinButton->GetValue());
			p.ignorePeriod = std::chrono::milliseconds((int)tickerIgnoreSpinButton->GetValue());
			std::variant<int, MusicalTickerParams> tickerParam = p;
			std::visit([this](auto &p){this->startCallback(0, "", "", p);}, tickerParam);
		} // Rest to be implemented
	}

	void setStartCallback(std::function<void(int, std::string, std::string, std::variant<int, MusicalTickerParams>)> startCallback) {
		this->startCallback = startCallback;
	}

	void updateThresholdValue() {
		char v[5];
		snprintf(v, 5, "%.3f", this->tickerThresholdScale->GetValue());
		this->tickerMusicThresholdValue->SetText(std::string(v));
	}

private:
	sfg::Box::Ptr box, glovesBox, periodicTickerBox, musicalTickerBox;
	sfg::ComboBox::Ptr serialComboLeft, serialComboRight, controllerDropdown, tickerDropdown, tickerMusicFileCombo;
	sf::RectangleShape background;
	sfg::SpinButton::Ptr tickerPeriodSpinButton, tickerLowFreqSpinButton, tickerHighFreqSpinButton, tickerAnalysisSpinButton, tickerIgnoreSpinButton;
	sfg::Scale::Ptr tickerThresholdScale;
	sfg::Label::Ptr tickerMusicThresholdValue;
	std::function<void(int, std::string, std::string, std::variant<int, MusicalTickerParams>)> startCallback;
	std::atomic_bool modelTraining = false;
	NewModelTrainer* modelTrainer;
	serialib* serial;
	sfg::Desktop *desktop;
	sf::RenderWindow *window;
	std::thread m_thread;
};

#endif // MENU_HPP