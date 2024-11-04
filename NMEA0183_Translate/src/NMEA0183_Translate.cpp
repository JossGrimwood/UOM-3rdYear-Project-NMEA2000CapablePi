#include "NMEA0183_Translate.h"

NMEA0183_Translate::NMEA0183_Translate() {
    directory = serialPort.findParentDirectory("NMEA0183_Translate");
    directoryNoSpaces = serialPort.addQuotesToDirectoriesWithSpaces(directory);
    if (!serialPort.openPort("COM5")) {
       exit(EXIT_FAILURE);
    }
}

NMEA0183_Translate::~NMEA0183_Translate() {
    if (!serialPort.openPort("COM5")) {
        return;
    }
}

void NMEA0183_Translate::handleMessage(std::string Message) {
    // Split the message into individual JSON strings
    std::vector<std::string> jsonStrings = SplitJSONStrings(Message);
    // Send each JSON string using sendData function
    for (const auto& jsonString : jsonStrings) {
        if (!serialPort.sendData(Translate(jsonString) + "\r\n")) {
            exit(EXIT_FAILURE);
        }
    }
}

std::vector<std::string> NMEA0183_Translate::SplitJSONStrings(const std::string& message) {
    std::vector<std::string> jsonMessages;
    int balance = 0;
    std::string currentMessage;

    for (char c : message) {
        if (c == '{') {
            balance++;
            currentMessage += c;
        } else if (c == '}') {
            balance--;
            currentMessage += c;
            if (balance == 0) {
                jsonMessages.push_back(currentMessage);
                currentMessage.clear();
            }
        } else {
            currentMessage += c;
        }
    }

    return jsonMessages;
}

std::string NMEA0183_Translate::Translate(const std::string& JSONdata) {
    //find field type
    std::string name;
    size_t startPos = JSONdata.find("\"description\":\"");
    if (startPos != std::string::npos) {
        startPos += sizeof("\"description\":\"") - 1;
        size_t endPos = JSONdata.find("\"", startPos);
        if (endPos != std::string::npos) {
            name = JSONdata.substr(startPos, endPos - startPos);
        }
    }

    //find refrence line for data
    std::ifstream file(( directory + "\\src\\TranslateLookup.txt"), std::ifstream::in );
    std::string line;
    std::string result;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line.find(name) == 0) { // If the line starts with 'name'
                // Remove 'name' from the line
                result = line.erase(0, name.length());
                break; // Exit the loop after finding the line
            }
        }
        file.close();
    } else {
        std::cerr << "Unable to open lookup file: " << directory + "\\src\\TranslateLookup.txt error: " << GetLastError()  << std::endl;
        return "$error";
    }

    std::regex pattern(R"(\{([\w\s]+)\})"); // Regular expression to match {<identifier>}
    std::smatch matches;

    // Find all matches of {<identifier>} in the line
    while (std::regex_search(result, matches, pattern)) {
        if (matches.size() > 1) {
            std::string identifier = matches[1]; // Extract the identifier
            // Create the search string "<identifier>":<value>
            std::string searchString = "\"" + identifier + "\":";
            // Find the position of searchString in JSONdata
            size_t pos = JSONdata.find(searchString);
            if (pos != std::string::npos) {
                size_t startPos = pos + searchString.length();
                // Search for terminating comma or }
                size_t endPos = JSONdata.find_first_of(",}", startPos);
                if (endPos != std::string::npos) {
                    std::string value = JSONdata.substr(startPos, endPos - startPos);
                    // Replace <identifier> with <value> in the result
                    result.replace(matches.position(), matches.length(), value);
                } else break;
            } else break;
        } else break;
    }

    return result;
}
