#pragma once

#include "IAsymmetricEncryption.h"

class ISecureConnection
{
public:

	virtual ~ISecureConnection() = default;

	/// @brief Создать безопасное соединение, в котором будет выработан сессионный ключ для сообщения и для хеша
	/// @param provider Интерфейс ассиметричного шифрования
	virtual void MakeSecureConnection(IAsymmetricEncryption* provider) = 0;

	/// @brief Отправить зашифрованное сообщение клиенту
	/// @param message Сообщение
	virtual void SendSecuredMessage(const std::string& message) = 0;

	/// @brief Получить сообщение от клиента
	/// @return Полученное сообщение
	virtual std::string RecieveMessage() = 0;
};