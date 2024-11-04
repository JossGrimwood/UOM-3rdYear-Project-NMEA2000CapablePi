#include "serialPort.h"

SerialPort::SerialPort() : hSerial(INVALID_HANDLE_VALUE), portName("") {
    directory = findParentDirectory("NMEA0183_Translate");
    std::cout << directory << std::endl;
    directoryNoSpaces = addQuotesToDirectoriesWithSpaces(directory);
    if (!isCom0comInstalled()){
        exit(EXIT_FAILURE);
    }
}

SerialPort::~SerialPort() {
    closePort();
}

bool SerialPort::openPort(const std::string& port) {
    portName = port;
    if (!createVirtualPort(port)) {
            std::cerr << "Error: Unable to create virtual COM port" << std::endl;
            return false;
    }

    hSerial = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Unable to open serial port error = " << GetLastError() << std::endl;
        return false;
    }

    DCB dcbSerialParams = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    // Set serial port parameters
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Unable to get serial port state." << std::endl;
        CloseHandle(hSerial);
        return false;
    }
    dcbSerialParams.BaudRate = CBR_4800;   // Set baud rate
    dcbSerialParams.ByteSize = 8;          // Set data bits
    dcbSerialParams.StopBits = ONESTOPBIT;  // Set stop bits
    dcbSerialParams.Parity = NOPARITY;      // Set parity

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "Error: Unable to set serial port parameters." << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    // Set timeouts
    timeouts.ReadIntervalTimeout = 100;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.ReadTotalTimeoutMultiplier = 20;
    timeouts.WriteTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 20;
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "Error: Unable to set timeouts." << std::endl;
        CloseHandle(hSerial);
        return false;
    }

    return true;
}

bool SerialPort::sendData(const std::string& data) {
    if (hSerial == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: Serial port is not open" << std::endl;
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hSerial, data.c_str(), data.length(), &bytesWritten, NULL)) {
        std::cerr << "Error: Unable to write to serial port." << std::endl;
        return false;
    }
    //std::cout << "Data written to serial port successfully." << std::endl;
    
    return true;
}

void SerialPort::closePort() {
    if (hSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(hSerial);
        hSerial = INVALID_HANDLE_VALUE;
    }
}

std::string SerialPort::addQuotesToDirectoriesWithSpaces(const std::string& directoryName) {
    std::string result;

    for (char c : directoryName) {
        if (c == ' ') {
            result.push_back('"');
            result.push_back(' ');
            result.push_back('"');
        } else {
            result.push_back(c);
        }
    }
    //std::cout << result << std::endl;
    return result;
}

std::string SerialPort::findParentDirectory(const std::string& directoryName) {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName( NULL, buffer, MAX_PATH );
    //std::cout << "location:" << buffer << std::endl;

    #ifdef UNICODE
        int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
        std::string result(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &result[0], size, nullptr, nullptr);
        result;
    #else
        std::string result;
        result = std::string(buffer);
    #endif
    result = result.substr(0, result.find_last_of("\\"));
    return result.substr(0, result.find_last_of("\\"));
}

bool SerialPort::createVirtualPort(const std::string& port) {
    char targetPath[MAX_PATH];
    //check port exists
    if (QueryDosDevice(port.c_str(), targetPath, MAX_PATH) == 0){
        // Command to install virtual COM port pair
        // std::string installCommand = directoryNoSpaces + "\\bin\\setupc.exe install PortName=" + port + " PortName=" + port + "out";

        // if (system(installCommand.c_str()) != 0) {
        //     return false;
        // }
        if (!SetCurrentDirectory((directory + "\\bin").c_str())) {
            std::cerr << "Error: Failed to set current directory to " << directory << ". Error code: " << GetLastError() << std::endl;
            return false;
        }

        std::string params = "install PortName=" + port + " PortName=" + port + "out";
        std::wstring paramsWstr(params.begin(), params.end());
        std::string Path = directory + "\\bin\\setupc.exe";
        std::wstring PathWstr(Path.begin(), Path.end());

        //WINDOWS execute code
        SHELLEXECUTEINFOW shellExecInfo = { 0 };
        shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
        shellExecInfo.lpFile = PathWstr.c_str();
        shellExecInfo.lpParameters = paramsWstr.c_str();
        shellExecInfo.nShow = SW_SHOW;
        shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        if (ShellExecuteExW(&shellExecInfo)) {
            // Wait for the Install
            WaitForSingleObject(shellExecInfo.hProcess, INFINITE);
            CloseHandle(shellExecInfo.hProcess);

            // Check the exit code of the process
            DWORD exitCode;
            GetExitCodeProcess(shellExecInfo.hProcess, &exitCode);
            if (exitCode == 1223) {
                // User cancelled the operation
                std::cerr << "Installation cancelled by user" << std::endl;
                return false;
            } else if (exitCode != 0 && exitCode != 19988992) {//19988992 unsure why this code but it works but gives this error
                std::cerr << "Error: Installer exited with code " << exitCode << std::endl;
                return false;
            }

            return true;
        } else {
            std::cerr << "Error: Failed to execute the installer. Error code: " << GetLastError() << std::endl;
            return false;
        }
    }
    
    return true;
}

bool SerialPort::isCom0comInstalled() {
    std::string filePath = directory + "\\bin\\setupc.exe";
    std::ifstream file(filePath);
    if (!file.good()){
        //convert paths to windows format
        std::string params = "/S /D=" + directory + "\\bin";
        std::wstring paramsWstr(params.begin(), params.end());
        std::string Path = directory + "\\rsc\\Setup_com0com_v3.0.0.0_W7_x64_signed.exe";
        std::wstring PathWstr(Path.begin(), Path.end());

        //WINDOWS execute code
        SHELLEXECUTEINFOW shellExecInfo = { 0 };
        shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
        shellExecInfo.lpFile = PathWstr.c_str();
        shellExecInfo.lpParameters = paramsWstr.c_str();
        shellExecInfo.nShow = SW_SHOW;
        shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        if (ShellExecuteExW(&shellExecInfo)) {
            // Wait for the Install
            WaitForSingleObject(shellExecInfo.hProcess, INFINITE);
            CloseHandle(shellExecInfo.hProcess);

            // Check the exit code of the process
            DWORD exitCode;
            GetExitCodeProcess(shellExecInfo.hProcess, &exitCode);
            if (exitCode == 1223) {
                // User cancelled the operation
                std::cerr << "Installation cancelled by user" << std::endl;
                return false;
            } else if (exitCode != 0 && exitCode != 6684016) {//6684016 unsure why this code but it works but gives this error
                std::cerr << "Error: Installer exited with code " << exitCode << std::endl;
                return false;
            }

            return true;
        } else {
            std::cerr << "Error: Failed to execute the installer. Error code: " << GetLastError() << std::endl;
            return false;
        }
    }
    return true;
}
