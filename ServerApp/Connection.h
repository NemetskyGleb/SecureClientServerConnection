#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>

/// @brief Класс для создания защищенного соеденения на стороне сервера
class Connection
{
public:
	Connection();

	/// @brief Вывести в консоль hex представление ключа
	/// @param message Сообщение
	/// @param key Ключ
	void LogKey(const std::string& message, const std::string& key);

	/// @brief Создать RSA соединение, в котором будет выработан сессионный ключ для сообщения и для хеша
	void RSAConnection();

	/// @brief Отправить зашифрованное сообщение клиенту
	/// @param message Сообщение
	void SendSecuredMessage(const std::string& message);

	/// @brief Получить сообщение от клиента
	/// @return Полученное сообщение
	std::string RecieveMessageFromClient();

	~Connection() {}
private:
	CryptoPP::RSA::PrivateKey privateKey_;
	
	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	ServerSocket socket_;
	CryptoPP::HexEncoder logger_;
};