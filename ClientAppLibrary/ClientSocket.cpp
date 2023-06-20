#include "ClientSocket.h"
#include <iostream>

// Разделитель для сообщений из буфера
const std::string delimiter = "/sep/";

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

void ClientSocket::Send(const std::string& message) const
{
    std::string sendingMessage = message + delimiter;
    int32_t bytesSent = send(connectSocket_, sendingMessage.c_str(), sendingMessage.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        throw std::runtime_error("send failed");
    }
    std::cout << "Bytes sent: " << bytesSent << "\n";
}

std::string ClientSocket::WaitForResponse()
{
    if (!pendingMessages.empty())
    {
        auto message = pendingMessages.front();
        pendingMessages.pop_front();
        return message;
    }

	size_t bytesRecieved = 0;
	bytesRecieved = recv(connectSocket_, &recvbuf_[0], recvbuf_.capacity(), 0);
	if (bytesRecieved > 0)
	{
		std::cout << "Bytes recieved " << bytesRecieved << std::endl;

		auto dataRecieved = std::string{ recvbuf_.data(), recvbuf_.data() + bytesRecieved };

		size_t pos = 0;
		std::string message;
		while ((pos = dataRecieved.find(delimiter)) != std::string::npos) {
			message = dataRecieved.substr(0, pos);
			dataRecieved.erase(0, pos + delimiter.length());
			pendingMessages.push_back(std::move(message));
		}
		auto firstMessage = pendingMessages.front();
		pendingMessages.pop_front();

		return firstMessage;
	}
	else if (bytesRecieved == 0)
	{
		std::cerr << "Connection closed\n";
		return std::string();
	}
	else
		std::cerr << "recv failed with error: " << WSAGetLastError() << '\n';
}

ClientSocket::~ClientSocket()
{
    shutdown(connectSocket_, SD_BOTH);
    closesocket(connectSocket_);
	WSACleanup();
}
