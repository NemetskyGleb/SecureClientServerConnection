#include "ClientSocket.h"
#include <iostream>

void ClientSocket::MakeConnection()
{
	checkRetVal(getaddrinfo(serverName_.c_str()
				, port_.c_str()
				, &hints_
				, &result_), "getaddrinfo failed");

    // Attempt to connect to an address until one succeeds
    // Taking addrinfo linked list from result
    for (struct addrinfo* ptr = result_; ptr != nullptr; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        connectSocket_ = socket(ptr->ai_family,
            ptr->ai_socktype,
            ptr->ai_protocol);
        if (connectSocket_ == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            throw std::runtime_error("socket failed");
        }

        // Connect to server.
        int iResult = connect(connectSocket_, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket_);
            connectSocket_ = INVALID_SOCKET;
            continue;
        }
        break;
    }
    freeaddrinfo(result_);

    if (connectSocket_ == INVALID_SOCKET) {
        throw std::runtime_error("Unable to connect to server");
    }
    
    std::cout << "Connection with server " << serverName_ << " succeed." << std::endl;    
}

void ClientSocket::Send(const std::string& message)
{
    int32_t bytesSent = send(connectSocket_, message.c_str(), message.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        throw std::runtime_error("send failed");
    }
    std::cout << "Bytes sent: " << bytesSent << "\n";
}

std::string ClientSocket::WaitForResponse()
{
    int32_t bytesRecieved = 0;
    do {
        bytesRecieved = recv(connectSocket_, &recvbuf_[0], recvbuf_.capacity(), 0);
        if (bytesRecieved > 0)
        {
            std::cout << "Bytes recieved: " << bytesRecieved << '\n';
            return std::string{ recvbuf_.data(), recvbuf_.data() + bytesRecieved };
        }
        else if (bytesRecieved == 0)
            std::cerr << "Connection closed\n";
        else
            std::cerr << "recv failed with error: " << WSAGetLastError() << '\n';
    } while (bytesRecieved > 0);
}

ClientSocket::~ClientSocket()
{
    shutdown(connectSocket_, SD_BOTH);
    closesocket(connectSocket_);
	WSACleanup();
}
