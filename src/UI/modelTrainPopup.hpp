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

namespace fs = std::filesystem;

class NewModelTrainer {
public:
	NewModelTrainer(serialib &serial, std::string savepath, sf::Vector2f windowSize, sfg::Desktop *desktop, std::function<void()> onTrainingComplete) : serial(serial), savepath(savepath), windowSize(windowSize), desktop(desktop), onTrainingComplete(onTrainingComplete) {
		background = sf::RectangleShape(sf::Vector2f(windowSize.x / 10, windowSize.y / 5));
		background.setFillColor(sf::Color(0xA67C52FF));
		background.setOutlineThickness(10);
		background.setOutlineColor(sf::Color(0x865C32FF));
		
		font.loadFromMemory(&Roboto_Italic_ttf, Roboto_Italic_ttf_len);
		textbox.setFont(font);
		textbox.setFillColor(sf::Color::White);
		textbox.setCharacterSize(16);
		textboxBorder = sf::RectangleShape(sf::Vector2f(windowSize.x / 10, windowSize.y / 5));
		textboxBorder.setFillColor(sf::Color(0x00000000));
		textboxBorder.setOutlineThickness(5);
		textboxBorder.setOutlineColor(sf::Color(0x865C32FF));

		label = sfg::Label::Create("Enter the name for the model:");
		desktop->Add(label);

		btn = sfg::Button::Create("Start");
		desktop->Add(btn);
		btn->GetSignal(sfg::Button::OnLeftClick).Connect(
			std::bind(&NewModelTrainer::btnPressCallback, this)
		);
	}

	void render(sf::RenderWindow& window) {
		float x = 4*windowSize.x/10;
		float y = 4*windowSize.y/10;
		float w = windowSize.x - x*2;
		float h = windowSize.y - y*2;

		background.setPosition(sf::Vector2f(x, y));
		background.setSize(sf::Vector2f(w, h));
		window.draw(background);

		x += 15;
		y += 10;
		w -= 30;
		float lh = h / 2 - 10;

		label->SetPosition(sf::Vector2f(x, y));
		label->SetRequisition(sf::Vector2f(w, lh));

		y += lh + 10;

		textbox.setPosition(sf::Vector2f(x, y));
		textboxBorder.setPosition(sf::Vector2f(x, y));
		textboxBorder.setSize(sf::Vector2f(w, lh / 2));

		if (stage == 0) {
			window.draw(textbox);
			window.draw(textboxBorder);
		}
		
		y += lh / 2 + 10;
		h = h / 3 - 20;
		h -= 10;
		
		btn->SetPosition(sf::Vector2f(x, y));
		btn->SetRequisition(sf::Vector2f(w, h));
	}

	void update(sf::Event &event) {
		if (stage > 0 && stage < 5) {
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
	
	void btnPressCallback() {
		if (stage == 0)
			stage++;
		if (stage == 1) {
			char v[3];
			sprintf(v, "%d", stage - 1);
			label->SetText("Ready the gesture for " + std::string(v) + " on\nyour hand and press the\nspacebar to start recording\nfor 10 seconds.");
			btn->Show(false);
		}
		if (stage == 5) {
			label->SetText("Please wait...\nThe model is training...");
			btn->Show(false);
			if (m_thread.joinable()) {
				m_thread.join();
			}
			m_thread = std::thread(std::bind(&NewModelTrainer::trainAndSaveModel, this));
		}
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
