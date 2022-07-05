#include <iostream>
#include <string>

#if defined _WIN32

#include <Windows.h>
#include <vector>
#include <string>

std::vector<std::string> SelectComPort() //added function to find the present serial 
{
    std::vector<std::string> com_ports;
    char lpTargetPath[5000]; // buffer to store the path of the COMPORTS
    bool gotPort = false; // in case the port is not found

    for (int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
        DWORD test = QueryDosDevice(str.c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (test != 0) //QueryDosDevice returns zero if it didn't find an object
        {
            std::cout << str << ": " << lpTargetPath << std::endl;
            gotPort = true;
            com_ports.push_back(str);
        }

        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
        }
    }

    if (!gotPort)
    {
        std::cout << "No COM ports found" << std::endl;
    }

    return com_ports;
}

#elif defined __linux__

#error "Linux is not supported yet"

#endif