/*
 * Filename: modelTrainPopup.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Class for displaying a popup for training and saving a new model. It is designed to be invoked from the menu screen.
 */

#if !defined(MODELTRAINPOPUP_HPP)
#define MODELTRAINPOPUP_HPP

#include "../gesture/gestureModel.hpp"
#include "../gloveInterface/recordPose.hpp"
#include "Roboto_Italic_Font.hpp"
#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <functional>
#include <future>

// Create a short alias for the filesystem module
namespace fs = std::filesystem;

// NewModelTrainer class displays a "popup" that allows a user to record hand gesture poses, 
class NewModelTrainer {
public:
	// The NewModelTrainer requires the serial port to read data from, the path to save the model in, the renderwindow size, the SFGUI desktop object and the callback for when the training is completed.
	NewModelTrainer(serialib &serial, std::string savepath, sf::Vector2f windowSize, sfg::Desktop *desktop, std::function<void()> onTrainingComplete) : serial(serial), savepath(savepath), windowSize(windowSize), desktop(desktop), onTrainingComplete(onTrainingComplete) {
		// Set a background for the popup
		background = sf::RectangleShape(sf::Vector2f(windowSize.x / 10, windowSize.y / 5));
		background.setFillColor(sf::Color(0xA67C52FF));
		background.setOutlineThickness(10);
		background.setOutlineColor(sf::Color(0x865C32FF));
		
		// Load the font and set the textbox
		font.loadFromMemory(&Roboto_Italic_ttf, Roboto_Italic_ttf_len);
		textbox.setFont(font);
		textbox.setFillColor(sf::Color::White);
		textbox.setCharacterSize(16);
		textboxBorder = sf::RectangleShape(sf::Vector2f(windowSize.x / 10, windowSize.y / 5));
		textboxBorder.setFillColor(sf::Color(0x00000000));
		textboxBorder.setOutlineThickness(5);
		textboxBorder.setOutlineColor(sf::Color(0x865C32FF));

		// Create an SFGUI label to show instructions
		label = sfg::Label::Create("Enter the name for the model:");
		desktop->Add(label);

		// Create an SFGUI button to proceed through the stages
		btn = sfg::Button::Create("Start");
		desktop->Add(btn);
		btn->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&NewModelTrainer::btnPressCallback, this)
		);
	}

	// To render the elements of the popup
	void render(sf::RenderWindow& window) {
		// Calculate the desired position and size of the popup
		float x = 4*windowSize.x/10;
		float y = 4*windowSize.y/10;
		float w = windowSize.x - x*2;
		float h = windowSize.y - y*2;

		// Set the background position and size and draw it to the window
		background.setPosition(sf::Vector2f(x, y));
		background.setSize(sf::Vector2f(w, h));
		window.draw(background);

		// Calculate the desired position and size of the label
		x += 15;
		y += 10;
		w -= 30;
		float lh = h / 2 - 10;

		// Set the label position and size
		label->SetPosition(sf::Vector2f(x, y));
		label->SetRequisition(sf::Vector2f(w, lh));

		// Update the desired y position for the textbox
		y += lh + 10;

		// Set the textbox position and size
		textbox.setPosition(sf::Vector2f(x, y));
		textboxBorder.setPosition(sf::Vector2f(x, y));
		textboxBorder.setSize(sf::Vector2f(w, lh / 2));

		// Draw the textbox only when needed (stage 0)
		if (stage == 0) {
			window.draw(textbox);
			window.draw(textboxBorder);
		}
		
		// Update y and h for the button
		y += lh / 2 + 10;
		h = h / 3 - 20;
		h -= 10;
		
		// Set the button position and size
		btn->SetPosition(sf::Vector2f(x, y));
		btn->SetRequisition(sf::Vector2f(w, h));
	}

	// Update function to handle GUI events
	void update(sf::Event &event) {
		if (stage > 0 && stage < 5) {
			// For stages 1-4, start recording a pose when space is pressed
			if (event.type == sf::Event::KeyPressed)
				if (event.key.code == sf::Keyboard::Space)
					spacePressed = true;
			if (event.type == sf::Event::KeyReleased && spacePressed) {
				if (event.key.code == sf::Keyboard::Space && spacePressed) {
					spacePressed = false;
					if (!recordingPose) {
						recordingPose = true;
						if (m_thread.joinable()) {
							m_thread.join();
						}
						m_thread = std::thread(std::bind(&NewModelTrainer::recordNewPose, this));
					}
				}
			}
		} else if (stage == 0) {
			// For stage 0, add entered text to the textbox           
			if (event.type == sf::Event::TextEntered) {
				auto unicode = event.text.unicode;
				if ((unicode >= 48 && unicode <= 57) || (unicode >= 65 && unicode <= 90) || (unicode >= 97 && unicode <= 122) || unicode == 95) {
					input += static_cast<char>(unicode);
					textbox.setString(input);
				}
				if (unicode == 8) {
					input = input.substr(0, input.size() - 1);
					textbox.setString(input);
				}
			}
		}
	}

	// Function to record a new pose, run in a separate thread
	void recordNewPose() {
		char v[3];
		sprintf(v, "%d", stage - 1);
		label->SetText("Recording the gesture for " + std::string(v) + "...");
		auto new_poses = recordPose(serial);
		for (auto &i: new_poses) {
			poses.push_back(std::vector<float>(i.begin(), i.end()));
			labels.push_back(stage-1);
		}
		stage++;
		if (stage == 5) {
			label->SetText("Press start to train and\nsave the model.");
			btn->Show(true);
			btn->SetLabel("Start");
			return;
		}
		sprintf(v, "%d", stage - 1);
		label->SetText("Ready the gesture for " + std::string(v) + " on\nyour hand and press the\nspacebar to start recording\nfor 10 seconds.");
		btn->Show(false);
		recordingPose = false;
	}

	// Function to train and save the gesture model, run in a separate thread
	void trainAndSaveModel() {
		std::cout << "Training model\n";
		trainer_t t = createGestureModel();
		std::vector<sample_type> samples = convertToSampleTypes(poses);
		df_t df = trainGestureModel(t, samples, labels);
		if (!fs::is_directory(fs::path(savepath)))
			fs::create_directories(fs::path(savepath));
		saveGestureModel(df, savepath + input);
		stage++;
		label->SetText("Model trained and saved.\nPress done to continue.");
		btn->Show(true);
		btn->SetLabel("Done.");
	}
	
	// Call back function for button press
	void btnPressCallback() {
		// For stage 0, just increment the stage
		if (stage == 0)
			stage++;
		// For stage 1, move to the recording stages
		if (stage == 1) {
			char v[3];
			sprintf(v, "%d", stage - 1);
			label->SetText("Ready the gesture for " + std::string(v) + " on\nyour hand and press the\nspacebar to start recording\nfor 10 seconds.");
			btn->Show(false);
		}
		// For stage 5, start model training show text to wait for the model to be trained and saved
		if (stage == 5) {
			label->SetText("Please wait...\nThe model is training...");
			btn->Show(false);
			if (m_thread.joinable()) {
				m_thread.join();
			}
			m_thread = std::thread(std::bind(&NewModelTrainer::trainAndSaveModel, this));
		}
		// For stage 6, remove all GUI elements and call the callback function for completion of training and saving the model
		if (stage == 6) {
			desktop->Remove(label);
			desktop->Remove(btn);
			label.reset();
			btn.reset();
			onTrainingComplete();
		}
	}

private:
	sf::RectangleShape background, textboxBorder;
	sf::Font font;
	sf::Text textbox;
	std::string input = "";
	bool spacePressed = false, recordingPose = false;;
	sfg::Label::Ptr label;
	sfg::Button::Ptr btn;
	int stage = 0;
	serialib serial;
	std::vector<std::vector<float>> poses;
	std::vector<float> labels;
	std::string savepath;
	sf::Vector2f windowSize;
	sfg::Desktop *desktop;
	std::function<void()> onTrainingComplete;
	std::thread m_thread;
};

#endif // MODELTRAINPOPUP_HPP
