/*
 * Filename: availableSerial.hpp
 * Author: Malolan Venkataraghavan
 * 
 * Function to list all available serial ports. Currently only implemented for Windows.
 */

#include <iostream>
#include <string>
#include <vector>

// Windows implementation
#if defined _WIN32

#include <Windows.h>

// Function to list all available serial ports. Currently only implemented for Windows.
std::vector<std::string> SelectComPort()
{
    // To store the list of serial ports.
    std::vector<std::string> com_ports;
    // Buffer to store the path of the COMPORTS.
    char lpTargetPath[5000];
    // Boolean variable indicating whether a port was found.
    bool gotPort = false;

    // Check ports from COM0 to COM255
    for (int i = 0; i < 255; i++) {
        // Convert from integer 0, 1, 2... to strings COM0, COM1, COM2...
        std::string str = "COM" + std::to_string(i);

        // Try opening the COM port.
        DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

        // Check if the return value is not 0. Return value 0 indicates there is no device in that COM port.
        if (test != 0) {
            std::cout << str << ": " << lpTargetPath << std::endl;
            gotPort = true;
            com_ports.push_back(str);
        }
    }

    // If no port was found, log it to the command line.
    if (!gotPort) {
        std::cout << "No COM ports found" << std::endl;
    }

    // Return the list of COM ports.
    return com_ports;
}

// Linux implementation.
#elif defined __linux__

// Function not yet implemented for Linux.
#error "Linux is not supported yet"

#endif