#pragma once

#include <string>

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
