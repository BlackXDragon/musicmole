#include <serialib.h>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Usage: serialtest <port>" << std::endl;
		return 1;
	}
	serialib serial;


	// Connection to serial port
    char errorOpening = serial.openDevice(argv[1], 115200);


    // If connection fails, return the error code otherwise, display a success message
    if (errorOpening!=1) return errorOpening;
    printf ("Successful connection to %s\n",argv[1]);

	for (int i = 0; i < 10; i++) {
		while (!serial.available());
		// Read the serial port until the end of the line
		char buffer[256];
		serial.readString(buffer, '\n', 256);

		std::cout << "Received: " << buffer;

		serial.flushReceiver();
	}

    // Close the serial device
    serial.closeDevice();

	return 0;
}