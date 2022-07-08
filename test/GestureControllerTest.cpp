/*
 * Code to test the gesture recognition model on live data from serial.
 */

#include <iostream>
#include <signal.h>
#include "../src/controllers/GestureController.hpp"
#include "../src/gloveInterface/recordPose.hpp"

std::string s = "Show the pose in your hands";

void callback(int x, int y) {
	char xv[3], yv[3];
	sprintf(xv, "%d", x);
	sprintf(yv, "%d", y);
	s = "You showed: " + std::string(xv) + ", " + std::string(yv);
}

serialib lserial, rserial;

// SIGINT handler.
void signal_handler(int signal) {
	lserial.closeDevice();
	rserial.closeDevice();
	std::cout << "Exiting..." << std::endl;
	exit(0);
}

int main(int argc, char* argv[]) {
	// Check if the user has provided serial ports.
	if (argc < 3) {
		std::cout << "Please provide two serial ports." << std::endl;
		return 1;
	}
	
	// Try to open the serial ports.
	char err;
	try {
		err = lserial.openDevice(argv[1], 9600);
	} catch (std::exception& e) {
		std::cout << "Error opening serial port: " << e.what() << std::endl;
		return 1;
	}

	if (err != 1) {
		std::cout << "Error opening serial port: " << err << std::endl;
		return err;
	}

	try {
		err = rserial.openDevice(argv[1], 9600);
	} catch (std::exception& e) {
		std::cout << "Error opening serial port: " << e.what() << std::endl;
		return 1;
	}

	if (err != 1) {
		std::cout << "Error opening serial port: " << err << std::endl;
		return err;
	}

	// Set the SIGINT handler.
	signal(SIGINT, signal_handler);

	// Record poses
	std::vector<std::vector<float>> ldata, rdata;
	std::vector<float> llabels, rlabels;
	for (int i = 0; i < 4; i++) {
		char c;
		std::cout << "Press enter when you're ready to start recording pose " << i << " for the left hand for 10 secs.\n";
		std::cin >> c;
		std::vector<std::vector<double>> d = recordPose(lserial);
		ldata.push_back(std::vector<float>(d.begin(), d.end()));
		std::cout << "Left pose " << i << " recorded.\n";
		
		std::cout << "Press enter when you're ready to start recording pose " << i << " for the right hand for 10 secs.\n";
		std::cin >> c;
		d = recordPose(rserial);
		rdata.push_back(std::vector<float>(d.begin(), d.end()));
		std::cout << "Right pose " << i << " recorded.\n";
	}

	// Convert data to sample types
	std::vector<sample_type> lsamples, rsamples;
	lsamples = convertToSampleTypes(ldata);
	rsamples = convertToSampleTypes(rdata);

	// Create and train the left and right models
	auto ltrainer = createGestureModel();
	auto rtrainer = createGestureModel();
	auto ldf = trainGestureModel(ltrainer, lsamples, llabels);
	auto rdf = trainGestureModel(rtrainer, rsamples, rlabels);

	// Print status.
	std::cout << "Trained gesture recognition models." << std::endl;

	// Create SFML window
	sf::RenderWindow window(sf::VideoMode(800, 600), "Numerical Controller Test");
	sf::Font font;
	font.loadFromFile("Roboto-Italic.ttf");
	sf::Text text;
	text.setFont(font);
	text.setFillColor(sf::Color::White);
	text.setCharacterSize(24);

	sf::Vector2u windowSize = window.getSize();

	// Create Gesture Controller
	GestureController controller = GestureController(lserial, ldf, rserial, rdf, callback);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			controller.run(event);
			if (event.type == sf::Event::Closed)
				window.close();
		}

		text.setString(s);
		auto textBounds = text.getLocalBounds();
		text.setPosition(sf::Vector2f(windowSize.x/2-(textBounds.width/2), windowSize.y/2-(textBounds.height/2)));

		window.clear();
		window.draw(text);
		window.display();
	}

	return 0;
}