/*
 * Filename: menu.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class for displaying a menu screen to select the controller and ticker parameters before starting the game.
 */

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

// Create a short alias for the filesystem module
namespace fs = std::filesystem;

#ifdef _WIN32
	// Variables for paths to the home/user folder, and folders to store the music and gesture model files on Windows
	std::string homepath = std::string(getenv("HOMEDRIVE")) + std::string(getenv("HOMEPATH"));
	std::string musicpath = homepath + "\\Documents\\musicmole\\Music";
	std::string lmodelpath = homepath + "\\Documents\\musicmole\\Models\\Left\\";
	std::string rmodelpath = homepath + "\\Documents\\musicmole\\Models\\Right\\";
#endif
#ifdef linux
	// Variables for paths to the home/user folder, and folders to store the music and gesture model files on Linux
	std::string homepath = std::string(getenv("HOME"));
	std::string musicpath = homepath + "/musicmole/Music";
	std::string lmodelpath = homepath + "/musicmole/Models/Left/";
	std::string rmodelpath = homepath + "/musicmole/Models/Right/";
#endif

// Struct for storing and passing the required variables for a Musical Ticker
typedef struct {
	std::string filename;
	double lowFreq, highFreq, threshold;
	std::chrono::microseconds analysisPeriod, ignorePeriod;
} MusicalTickerParams;

// Struct for storing and passing the required variables for a Gesture Controller
typedef struct {
	std::string lCOMport, lmodelPath;
	std::string rCOMport, rmodelPath;
} GestureControllerParams;

// MenuWindow class that shows the menu for selecting controller and ticker params and handles training new gesture models
class MenuWindow {
public:
	/// @brief The constructor constructs all the necessary GUI elements and sets callback functions and default values
	/// @param desktop The SFGUI desktop object
	/// @param window The SFML RenderWindow object
	MenuWindow(sfg::Desktop& desktop, sf::RenderWindow& window) {
		this->desktop = &desktop;
		this->window = &window;

		// Set the brown background for the menu screen
		background = sf::RectangleShape(sf::Vector2f(600, 600));
		background.setFillColor(sf::Color(0xA67C52FF));
		
		// Create the whole container boc and set size and position
		box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		box->SetRequisition(sf::Vector2f(600, 600));
		box->SetPosition(sf::Vector2f(200, 200));

		// Add the title label
		auto label = sfg::Label::Create("Music Mole Settings");
		box->Pack(label);

		// Set the font size of labels
		desktop.GetEngine().SetProperty("Label", "FontSize", 24);

		// Create and add a separator
		auto line = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		box->Pack(line);
		
		// Controller settings section
		auto controllerLabel = sfg::Label::Create("Controller Settings");
		box->Pack(controllerLabel);

		auto controllerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		
		auto controllerModeLabel = sfg::Label::Create("Contoller mode:");
		controllerBox->Pack(controllerModeLabel);

		// Combo Box for selecting the mode of controller: Numerical(Keypad) or Glove(Hand Gesture)		
		controllerDropdown = sfg::ComboBox::Create();
		controllerDropdown->AppendItem("Numerical");
		controllerDropdown->AppendItem("Glove");

		// Set the callback function for changing the combo box option
		controllerDropdown->GetSignal(sfg::ComboBox::OnSelect).Connect(
			std::bind(&MenuWindow::onControllerDropdownChange, this)
		);

		controllerBox->Pack(controllerDropdown);

		box->Pack(controllerBox);

		// Glove controller settings section
		glovesBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		// Split into two halves for the left and right glove		
		auto glovesBoxLeft = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);
		auto glovesBoxRight = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 5.f);

		auto glovesLabelLeft = sfg::Label::Create("Left Glove");
		auto glovesLabelRight = sfg::Label::Create("Right Glove");

		glovesBoxLeft->Pack(glovesLabelLeft);
		glovesBoxRight->Pack(glovesLabelRight);

		// Glove Serial Port section
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

		serialButtonLeft->GetSignal(sfg::Button::OnLeftClick).Connect(the serial port combo boxes
			std::bind(&MenuWindow::refreshSerialCombo, this, serialComboLeft)
		);

		serialButtonRight->GetSignal(sfg::Button::OnLeftClick).Connect(the serial port combo boxes
			std::bind(&MenuWindow::refreshSerialCombo, this, serialComboRight)
		);

		serialBoxLeft->Pack(serialButtonLeft);
		serialBoxRight->Pack(serialButtonRight);

		glovesBoxLeft->Pack(serialBoxLeft);
		glovesBoxRight->Pack(serialBoxRight);
		
		// Glove controller gesture recognition model section
		auto modelBoxLeft = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		auto modelBoxRight = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto modelLabelLeft = sfg::Label::Create("Gesture Model");
		auto modelLabelRight = sfg::Label::Create("Gesture Model");

		modelBoxLeft->Pack(modelLabelLeft);
		modelBoxRight->Pack(modelLabelRight);

		modelComboLeft = sfg::ComboBox::Create();
		modelComboRight = sfg::ComboBox::Create();

		modelBoxLeft->Pack(modelComboLeft);
		modelBoxRight->Pack(modelComboRight);

		auto modelRefreshButtonLeft = sfg::Button::Create("Refresh");
		auto modelRefreshButtonRight = sfg::Button::Create("Refresh");

		modelRefreshButtonLeft->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshModelCombo, this, modelComboLeft, true)
		);

		modelRefreshButtonRight->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshModelCombo, this, modelComboRight, false)
		);

		modelBoxLeft->Pack(modelRefreshButtonLeft);
		modelBoxRight->Pack(modelRefreshButtonRight);

		auto modelDeleteButtonLeft = sfg::Button::Create("Delete");
		auto modelDeleteButtonRight = sfg::Button::Create("Delete");

		modelDeleteButtonLeft->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::deleteModel, this, modelComboLeft, true)
		);

		modelDeleteButtonRight->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::deleteModel, this, modelComboRight, false)
		);

		modelBoxLeft->Pack(modelDeleteButtonLeft);
		modelBoxRight->Pack(modelDeleteButtonRight);

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

		// Numerical controller does not need a section since it has no settings

		// Separator to separate controller and ticker settings sections
		line = sfg::Separator::Create(sfg::Separator::Orientation::HORIZONTAL);
		box->Pack(line);

		// Ticker Settings section
		auto tickerLabel = sfg::Label::Create("Ticker Settings");
		box->Pack(tickerLabel);

		auto tickerBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);
		
		auto tickerModeLabel = sfg::Label::Create("Ticker mode:");
		tickerBox->Pack(tickerModeLabel);
		
		// Combo Box to select ticker mode
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

		// Periodic ticker section
		auto tickerPeriodLabel = sfg::Label::Create("Ticker Period (s):");
		periodicTickerBox->Pack(tickerPeriodLabel);
		
		tickerPeriodSpinButton = sfg::SpinButton::Create(1, 15, 1);
		periodicTickerBox->Pack(tickerPeriodSpinButton);

		// Musical ticker section
		auto tickerMusicFileBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.f);

		auto tickerMusicFileLabel = sfg::Label::Create("Music:");
		tickerMusicFileCombo = sfg::ComboBox::Create();
		auto tickerMusicFileButton = sfg::Button::Create("Refresh");

		tickerMusicFileButton->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::refreshMusicCombo, this, tickerMusicFileCombo)
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

		// Start button to start the game
		auto startButton = sfg::Button::Create("Start!");
		box->Pack(startButton);

		startButton->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&MenuWindow::startGame, this)
		);

		// Reset to default values
		controllerDropdown->SelectItem(0);
		onControllerDropdownChange();
		tickerDropdown->SelectItem(0);
		onTickerDropdownChange();
		refreshMusicCombo(tickerMusicFileCombo);
		refreshModelCombo(modelComboLeft, true);
		refreshModelCombo(modelComboRight, false);
	}

	~MenuWindow() {}

	// Function to refresh the serial port combo boxes
	void refreshSerialCombo(sfg::ComboBox::Ptr combo) {
		while (combo->GetItemCount() > 0) {
			combo->RemoveItem(0);
		}
		auto availableSerialPorts = SelectComPort();
		for (auto& port : availableSerialPorts) {
			combo->AppendItem(port);
		}
	}

	// Function to refresh the gesture model combo boxes
	void refreshModelCombo(sfg::ComboBox::Ptr combo, bool left = true) {
		while (combo->GetItemCount() > 0) {
			combo->RemoveItem(0);
		}
		std::string modelpath = (left) ? lmodelpath : rmodelpath;
		if (!fs::is_directory(fs::path(modelpath)))
			fs::create_directories(fs::path(modelpath));
		for (auto& file : fs::directory_iterator(modelpath)) {
			combo->AppendItem(std::string(file.path().filename().u8string()));
		}
	}

	// Function to delete the gesture model selected in the combo boxes
	void deleteModel(sfg::ComboBox::Ptr combo, bool left = true) {
		std::string modelpath = (left) ? lmodelpath : rmodelpath;
		if (!fs::is_directory(fs::path(modelpath)))
			fs::create_directories(fs::path(modelpath));
		auto model = combo->GetSelectedText();
		if (model == "")
			return;
		if (!fs::exists(fs::path(modelpath + combo->GetSelectedText())))
			return;
		fs::remove(fs::path(modelpath + model));
		refreshModelCombo(modelComboLeft, true);
		refreshModelCombo(modelComboRight, false);
	}

	// Function to refresh the music files combo box
	void refreshMusicCombo(sfg::ComboBox::Ptr combo) {
		while (combo->GetItemCount() > 0) {
			combo->RemoveItem(0);
		}
		if (!fs::is_directory(fs::path(musicpath)))
			fs::create_directories(fs::path(musicpath));
		for (auto& file : fs::directory_iterator(musicpath)) {
			combo->AppendItem(std::string(file.path().filename().u8string()));
		}
	}
	
	// Function to update the size of the menu window given the size of the renderwindow
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

	// Function to draw the menu window on the renderwindow
	void Draw(sf::RenderWindow& window) {
		if (modelTraining) {
			box->Show(false);
			modelTrainer->render(window);
		} else {
			box->Show(true);
			window.draw(background);
		}
	}

	// Function to provide event update functionality to the new model trainer popup window
	void update(sf::Event& event) {
		if (modelTraining) {
			modelTrainer->update(event);
		}
	}

	// Function for showing a new model trainer popup when the button is clicked
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

	// Function to be called when the new model trainer has finished its job
	void modelTrainedCallback() {
		modelTraining = false;
		serial->closeDevice();
		refreshModelCombo(modelComboLeft, true);
		refreshModelCombo(modelComboRight, false);
	}

	// Function to return the Menu Window SFGUI box
	sfg::Widget::Ptr getWindow() {
		return box;
	}

	// Callback function for when the controller dropdown is changed
	void onControllerDropdownChange() {
		glovesBox->Show(true);
		if (this->controllerDropdown->GetSelectedItem() == 0)
			glovesBox->Show(false);
	}

	// Callback function for when the ticker dropdown is changed
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

	// Callback function for when the player presses the start game button
	void startGame() {
		int c = this->controllerDropdown->GetSelectedItem();
		int t = this->tickerDropdown->GetSelectedItem();
		if (c == 0 && t == 0) {
			int v = tickerPeriodSpinButton->GetValue();
			std::variant<int, MusicalTickerParams> tickerParam = v;
			std::variant<int, GestureControllerParams> controllerParam = 0;
			std::visit(this->startCallback, controllerParam, tickerParam);
		} else if (c == 0 && t == 1) {
			MusicalTickerParams tp = MusicalTickerParams{};
			#ifdef _WIN32
				tp.filename = musicpath + "\\" + tickerMusicFileCombo->GetSelectedText();
			#endif
			#ifdef linux
				tp.filename = musicpath + "/" + tickerMusicFileCombo->GetSelectedText();
			#endif
			tp.lowFreq = tickerLowFreqSpinButton->GetValue();
			tp.highFreq = tickerHighFreqSpinButton->GetValue();
			tp.threshold = tickerThresholdScale->GetValue();
			tp.analysisPeriod = std::chrono::milliseconds((int)tickerAnalysisSpinButton->GetValue());
			tp.ignorePeriod = std::chrono::milliseconds((int)tickerIgnoreSpinButton->GetValue());
			std::variant<int, MusicalTickerParams> tickerParam = tp;
			std::variant<int, GestureControllerParams> controllerParam = 0;
			std::visit(this->startCallback, controllerParam, tickerParam);
		} else if (c == 1 && t == 0) {
			int v = tickerPeriodSpinButton->GetValue();
			std::variant<int, MusicalTickerParams> tickerParam = v;
			GestureControllerParams cp = GestureControllerParams{};
			cp.lCOMport = serialComboLeft->GetSelectedText();
			cp.rCOMport = serialComboRight->GetSelectedText();
			cp.lmodelPath = lmodelpath + modelComboLeft->GetSelectedText();
			cp.rmodelPath = rmodelpath + modelComboRight->GetSelectedText();
			std::variant<int, GestureControllerParams> controllerParam = cp;
			std::visit(this->startCallback, controllerParam, tickerParam);
		} else {
			MusicalTickerParams tp = MusicalTickerParams{};
			#ifdef _WIN32
				tp.filename = musicpath + "\\" + tickerMusicFileCombo->GetSelectedText();
			#endif
			#ifdef linux
				tp.filename = musicpath + "/" + tickerMusicFileCombo->GetSelectedText();
			#endif
			tp.lowFreq = tickerLowFreqSpinButton->GetValue();
			tp.highFreq = tickerHighFreqSpinButton->GetValue();
			tp.threshold = tickerThresholdScale->GetValue();
			tp.analysisPeriod = std::chrono::milliseconds((int)tickerAnalysisSpinButton->GetValue());
			tp.ignorePeriod = std::chrono::milliseconds((int)tickerIgnoreSpinButton->GetValue());
			std::variant<int, MusicalTickerParams> tickerParam = tp;
			GestureControllerParams cp = GestureControllerParams{};
			cp.lCOMport = serialComboLeft->GetSelectedText();
			cp.rCOMport = serialComboRight->GetSelectedText();
			cp.lmodelPath = lmodelpath + modelComboLeft->GetSelectedText();
			cp.rmodelPath = rmodelpath + modelComboRight->GetSelectedText();
			std::variant<int, GestureControllerParams> controllerParam = cp;
			std::visit(this->startCallback, controllerParam, tickerParam);
		}
	}

	// To set the callback function to be called passing the params chosen to the main loop
	void setStartCallback(std::function<void(std::variant<int, GestureControllerParams>, std::variant<int, MusicalTickerParams>)> startCallback) {
		this->startCallback = startCallback;
	}

	// Function to update the label displaying the threshold value for the musical ticker
	void updateThresholdValue() {
		char v[5];
		snprintf(v, 5, "%.3f", this->tickerThresholdScale->GetValue());
		this->tickerMusicThresholdValue->SetText(std::string(v));
	}

private:
	sfg::Box::Ptr box, glovesBox, periodicTickerBox, musicalTickerBox;
	sfg::ComboBox::Ptr serialComboLeft, serialComboRight, controllerDropdown, tickerDropdown, tickerMusicFileCombo, modelComboLeft, modelComboRight;
	sf::RectangleShape background;
	sfg::SpinButton::Ptr tickerPeriodSpinButton, tickerLowFreqSpinButton, tickerHighFreqSpinButton, tickerAnalysisSpinButton, tickerIgnoreSpinButton;
	sfg::Scale::Ptr tickerThresholdScale;
	sfg::Label::Ptr tickerMusicThresholdValue;
	std::function<void(std::variant<int, GestureControllerParams>, std::variant<int, MusicalTickerParams>)> startCallback;
	std::atomic_bool modelTraining = false;
	NewModelTrainer* modelTrainer;
	serialib* serial;
	sfg::Desktop *desktop;
	sf::RenderWindow *window;
	std::thread m_thread;
};

#endif // MENU_HPP