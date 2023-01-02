#include "ServerSocket.h"
#include <iostream>
// https://learn.microsoft.com/ru-ru/windows/win32/winsock/winsock-server-application

void ServerSocket::MakeConnection()
{
	checkRetVal(getaddrinfo(NULL
		, port_.c_str()
		, &hints_
		, &result_), "getaddrinfo failed.");

	listenSocket_ = socket(result_->ai_family, result_->ai_socktype, result_->ai_protocol);
	if (listenSocket_ == INVALID_SOCKET)
	{
		throw std::runtime_error("socket failed");
	}

	// Setup the TCP listening socket
	checkRetVal(bind(listenSocket_, result_->ai_addr, (int)result_->ai_addrlen), "bind failed.");

	freeaddrinfo(result_);

	checkRetVal(listen(listenSocket_, SOMAXCONN), "listen failed" );
	
	clientSocket_ = accept(listenSocket_, NULL, NULL);
	if (clientSocket_ == INVALID_SOCKET) {
		closesocket(listenSocket_);
		throw std::runtime_error("accept failed.");
	}

	std::cout << "Connection established." << std::endl;

	closesocket(listenSocket_);
}

void ServerSocket::Send(const std::string& message) const
{
	int32_t bytesSent = send(clientSocket_, message.c_str(), message.size(), 0);
	if (bytesSent == SOCKET_ERROR) {
		closesocket(clientSocket_);
		printf("send failed with error: %d\n", WSAGetLastError());
		throw;
	}
	Sleep(100);
	std::cout << "Bytes sent: " << bytesSent << "\n";
}

std::string ServerSocket::WaitForRequest()
{
	size_t bytesRecieved = 0;
	do 
	{
		bytesRecieved = recv(clientSocket_, &recvbuf_[0], recvbuf_.capacity(), 0);
		if (bytesRecieved > 0) {
			std::cout << "Bytes recieved " << bytesRecieved << std::endl;
			return std::string{ recvbuf_.data(), recvbuf_.data() + bytesRecieved };
		}
		else if (bytesRecieved == 0)
		{
			std::cout << "Connection closing...\n";
			return std::string();
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			throw;
		}

	} while (bytesRecieved > 0);
}

ServerSocket::~ServerSocket()
{
	shutdown(clientSocket_, SD_SEND);

	closesocket(clientSocket_);
	WSACleanup();
}
