#include "ServerSocket.h"
#include <iostream>
// https://learn.microsoft.com/ru-ru/windows/win32/winsock/winsock-server-application

// ����������� ��� ��������� �� ������
const std::string delimiter = "/sep/";

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
	// sending message with separator
	std::string sendingMessage = message + delimiter;
	int32_t bytesSent = send(clientSocket_, sendingMessage.c_str(), sendingMessage.size(), 0);
	if (bytesSent == SOCKET_ERROR) {
		closesocket(clientSocket_);
		printf("send failed with error: %d\n", WSAGetLastError());
		throw;
	}
	std::cout << "Bytes sent: " << bytesSent << "\n";
}

std::string ServerSocket::WaitForRequest()
{
	if (!pendingMessages.empty())
	{
		auto message = pendingMessages.front();
		pendingMessages.pop_front();
		return message;
	}

	size_t bytesRecieved = 0;

	bytesRecieved = recv(clientSocket_, &recvbuf_[0], recvbuf_.capacity(), 0);
	if (bytesRecieved > 0) {
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
		std::cout << "Connection closing...\n";
		return std::string();
	}
	else {
		printf("recv failed with error: %d\n", WSAGetLastError());
		throw;
	}
}

ServerSocket::~ServerSocket()
{
	shutdown(clientSocket_, SD_SEND);

	closesocket(clientSocket_);
	WSACleanup();
}
