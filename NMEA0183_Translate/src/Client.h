#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#define _WIN32_WINNT 0x0501
#include <iostream>
#include <string>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include "NMEA0183_Translate.h"

class TCPClient {
private:
    SOCKET sockfd;
    sockaddr_in serverAddr;
    NMEA0183_Translate& translator;

public:
    TCPClient(NMEA0183_Translate& translator);
    ~TCPClient();
    
    void connectToServer(const std::string& hostname, const std::string& port);
    void receiveData();
};

#endif // TCP_CLIENT_H
