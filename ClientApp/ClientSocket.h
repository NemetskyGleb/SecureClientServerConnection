#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <string>
#include <stdexcept>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

/// @brief Сокет клиентского приложения
class ClientSocket
{
public:
	/// @brief Конструктор сокета
	/// @param port порт для формирования TCP соединения. По умолчанию 27015
	/// @param buflen длина буфера для получения сообщения. По умолчанию 512
	/// @param servername IP адрес сервера. По умолчанию localhost
	ClientSocket(const std::string& port = "27015",
		const std::string& servername = "localhost",
		size_t buflen = 512)
		: port_{ port }, serverName_{ servername }
	{
		recvbuf_.reserve(buflen);

		ZeroMemory(&hints_, sizeof(hints_));
		// could be IPv4 or IPv6
		hints_.ai_family = AF_UNSPEC;
		hints_.ai_socktype = SOCK_STREAM;
		hints_.ai_protocol = IPPROTO_TCP;

		// Initialize Winsock
		checkRetVal(WSAStartup(MAKEWORD(2, 2), &wsaData), "WSAStartup failed");
	}

	/// @brief Создать сокет соединения и осуществлить подключение с сервером
	void MakeConnection();
	
	/// @brief Отправить по сокету сообщение
	/// @param message Отправляемое сообщение
	void Send(const std::string& message) const;

	std::string WaitForResponse();

	~ClientSocket();

private:
	// данные узла сервера
	addrinfo* result_ = nullptr;
	// указатель для перебора связного списка
	addrinfo* ptr_ = nullptr;
	// данные узла клиента для подключения к серверу
	addrinfo hints_;
	
	// сокет соединения
	SOCKET connectSocket_ = INVALID_SOCKET;

	// версия WinSock
	WSADATA wsaData;

	/// port порт для формирования TCP соединения. 
	std::string port_;
	// IP адрес сервера
	std::string serverName_;
	// Буфер для получения сообщения
	std::string recvbuf_;

	inline int32_t checkRetVal(int32_t statusCode, const char* errorMessage) {
		if (statusCode == INVALID_SOCKET)
		{
			closesocket(connectSocket_);
			return statusCode;
		}
		else if (statusCode == SOCKET_ERROR)
		{
			closesocket(connectSocket_);
			throw std::runtime_error(std::string(errorMessage) + ".Socket error.");
		}
		else if (statusCode != 0)
		{
			throw std::runtime_error(std::string(errorMessage) + " Status code = " + std::to_string(statusCode));
		}
		return statusCode;
	}
};