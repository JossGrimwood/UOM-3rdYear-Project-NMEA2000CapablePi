#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include "SerialPort.h"

class NMEA0183_Translate {
private:
    SerialPort serialPort;
    std::string directory;
    std::string directoryNoSpaces;

    std::vector<std::string> SplitJSONStrings(const std::string& message);
    std::string Translate(const std::string& JSONdata);

public:
    NMEA0183_Translate();
    ~NMEA0183_Translate();

    void handleMessage(std::string Message);
};

