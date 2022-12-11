#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>
#include <stdexcept>

/// @brief 
class ServerSocket
{
public:
	/// @brief Конструктор сокета
	/// @param port порт для формирования TCP соединения. По умолчанию 27015
	/// @param buflen длина буфера для получения сообщения. По умолчанию 512
	/// @param servername IP адрес сервера. По умолчанию localhost
	ServerSocket(const std::string& port = "27015",
		size_t buflen = 512)
		: port_{ port }
	{
		recvbuf_.reserve(buflen);

		ZeroMemory(&hints_, sizeof(hints_));
		// IPv4 address
		hints_.ai_family = AF_INET;
		hints_.ai_socktype = SOCK_STREAM;
		hints_.ai_protocol = IPPROTO_TCP;
		hints_.ai_flags = AI_PASSIVE;

		// Initialize Winsock
		checkRetVal(WSAStartup(MAKEWORD(2, 2), &wsaData), "WSAStartup failed");
	}

	/// @brief Создать сокет для подключения и слушать по заданному порту
	void MakeConnection();

	/// @brief Отправить по сокету сообщение
	/// @param message Отправляемое сообщение
	void Send(const std::string& message);

	/// @brief Ждать сообщение с сервера
	/// @return Полученное сообщение 
	std::string WaitForRequest();

	~ServerSocket();

private:
	// данные узла
	addrinfo* result_ = nullptr;

	// настройки для узла сервера
	addrinfo hints_;

	// сокет для прослушивания
	SOCKET listenSocket_ = INVALID_SOCKET;
	// сокет клента после соединения
	SOCKET clientSocket_ = INVALID_SOCKET;

	// версия WinSock
	WSADATA wsaData;

	/// port порт для формирования TCP соединения. 
	std::string port_;

	// Буфер для получения сообщения
	std::string recvbuf_;

	// Проверка возвращаемого кода, приводящего к завершению программы
	inline int32_t checkRetVal(int32_t statusCode, const char* errorMessage) {
		if (statusCode == SOCKET_ERROR)
		{
			if (result_)
			{
				freeaddrinfo(result_);
			}
			closesocket(listenSocket_);
			throw std::runtime_error(std::string(errorMessage) + ".Socket error.");
		}
		else if (statusCode != 0)
		{
			throw std::runtime_error(std::string(errorMessage) + " Status code = " + std::to_string(statusCode));
		}
		return statusCode;
	}
};