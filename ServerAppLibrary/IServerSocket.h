#pragma once

#include <string>

const std::string DEFAULT_PORT = "27015";
const size_t DEFAULT_BUFLEN = 512;

/// @brief Настройки сокета
struct Settings
{
	std::string port = DEFAULT_PORT; /// порт для формирования TCP соединения. По умолчанию 27015
	size_t buflen = 512; // длина буфера для получения сообщения. По умолчанию 512
};

class IServerSocket
{
public:
	virtual ~IServerSocket() = default;
	
	/// @brief Создать сокет для подключения и слушать по заданному порту
	virtual void MakeConnection() = 0;

	/// @brief Отправить по сокету сообщение
	/// @param message Отправляемое сообщение
	virtual void Send(const std::string& message) const = 0;

	/// @brief Ждать сообщение с сервера
	/// @return Полученное сообщение 
	virtual std::string WaitForRequest() = 0;
};
