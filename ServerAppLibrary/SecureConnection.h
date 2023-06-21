#pragma once

#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include "ServerSocket.h"
#include "Logger.h"
#include "IAsymmetricEncryption.h"

#include <string>

/// @brief Класс для создания защищенного соединения и отправки, получения защищенных сообщений
class SecureConnection
{
public:
	SecureConnection(std::unique_ptr<ServerSocket> socket, std::shared_ptr<Logger> logger);

	~SecureConnection();

	/// @brief Создать безопасное соединение, в котором будет выработан сессионный ключ для сообщения и для хеша
	/// @param provider Интерфейс ассиметричного шифрования
	void MakeSecureConnection(IAsymmetricEncryption* provider);

	/// @brief Отправить зашифрованное сообщение клиенту
	/// @param message Сообщение
	void SendSecuredMessage(const std::string& message);

	/// @brief Получить сообщение от клиента
	/// @return Полученное сообщение
	std::string RecieveMessage();

private:

	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	std::unique_ptr<ServerSocket> socket_;
	std::shared_ptr<Logger> logger_;

	CryptoPP::SHA256 hash_;
};