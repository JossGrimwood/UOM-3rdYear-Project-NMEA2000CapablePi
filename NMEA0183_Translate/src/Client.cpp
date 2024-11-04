#include "Client.h"

TCPClient::TCPClient(NMEA0183_Translate& translator) : translator(translator), sockfd(INVALID_SOCKET) {}

TCPClient::~TCPClient() {
    if (sockfd != INVALID_SOCKET) {
        closesocket(sockfd);
    }
    WSACleanup();
}
    void TCPClient::connectToServer(const std::string& hostname, const std::string& port) {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed with error: " << result << std::endl;
            exit(EXIT_FAILURE);
        }

        struct addrinfo* resultAddr = nullptr;
        struct addrinfo hints;

        // Setup the hints address info structure
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Resolve the server address and port
        result = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &resultAddr);
        if (result != 0) {
            std::cerr << "getaddrinfo failed with error: " << result << std::endl;
            std::cerr << "ensure pi is running and connected to nmea200 backbone" << std::endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Attempt to connect to an address until one succeeds
        for (struct addrinfo* ptr = resultAddr; ptr != NULL; ptr = ptr->ai_next) {
            // Create a SOCKET for connecting to server
            sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (sockfd == INVALID_SOCKET) {
                std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
                std::cerr << "ensure pi is running and connected to nmea200 backbone" << std::endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }

            // Connect to server
            result = connect(sockfd, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));
            if (result == SOCKET_ERROR) {
                closesocket(sockfd);
                sockfd = INVALID_SOCKET;
                continue;
            }
            break;
        }

        freeaddrinfo(resultAddr);

        if (sockfd == INVALID_SOCKET) {
            std::cerr << "Unable to connect to server" << std::endl;
            std::cerr << "ensure pi is running and connected to nmea200 backbone" << std::endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        std::cout << "Connected to server" << std::endl;
    }

void TCPClient::receiveData() {
        char buffer[1024];
        int bytesRead;
        // Receive data from pi as json
        bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::cerr << "Error: Failed to receive data. Last Error: " << WSAGetLastError() << std::endl;
            closesocket(sockfd);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Null-terminate the received data
        buffer[bytesRead] = '\0';

        // handle message and send dat onto comport
        //std::cout <<std::string(buffer) << std::endl;
        translator.handleMessage(std::string(buffer));
    }

int main() {
    const std::string hostname = "nmea2000pi.local";
    const std::string port = "2598";
    NMEA0183_Translate translator;
    TCPClient client(translator);
    client.connectToServer(hostname, port);
    while (true) {
        client.receiveData();
    }
    return 0;
}


