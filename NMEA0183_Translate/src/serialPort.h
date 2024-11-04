#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <Windows.h>
#include <iostream>
#include <string>

#include <fstream>


class SerialPort {
private:
    HANDLE hSerial;
    std::string portName;
    std::string directory;
    std::string directoryNoSpaces;

public:
    SerialPort();
    ~SerialPort();

    bool openPort(const std::string& port);
    bool sendData(const std::string& data);
    void closePort();
    std::string addQuotesToDirectoriesWithSpaces(const std::string& directoryName);
    std::string findParentDirectory(const std::string& directoryName);

private:
    bool createVirtualPort(const std::string& port);
    bool isCom0comInstalled();
};

#endif // SERIALPORT_H
