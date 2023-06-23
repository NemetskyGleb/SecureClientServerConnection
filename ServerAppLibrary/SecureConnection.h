#pragma once

#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include "Logger.h"

#include "IServerSocket.h"
#include "ISecureConnection.h"
#include "IAsymmetricEncryption.h"
#include "ISymmetricEncryption.h"
#include "HashCalculator.h"

#include <string>

/// @brief Класс для создания защищенного соединения и отправки, получения защищенных сообщений
class SecureConnection : public ISecureConnection
{
public:
	SecureConnection(std::unique_ptr<IServerSocket> socket,
					 ISymmetricEncryption* symmetricEncryptor,
				     std::shared_ptr<Logger> logger);

	~SecureConnection();

	/// @brief Создать безопасное соединение, в котором будет выработан сессионный ключ для сообщения и для хеша
	/// @param provider Интерфейс ассиметричного шифрования
	void MakeSecureConnection(IAsymmetricEncryption* provider) override;

	/// @brief Отправить зашифрованное сообщение клиенту
	/// @param message Сообщение
	void SendSecuredMessage(const std::string& message) override;

	/// @brief Получить сообщение от клиента
	/// @return Полученное сообщение
	std::string RecieveMessage() override;

protected:
	std::string EncryptOnSessionKey(const std::string& message, 
								    const CryptoPP::SecByteBlock& key,
									const CryptoPP::SecByteBlock& iv);

	std::string DecryptWithSessionKey(const std::string& message,
									  const CryptoPP::SecByteBlock& key,
									  const CryptoPP::SecByteBlock& iv);
private:
	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	std::unique_ptr<IServerSocket> socket_;
	std::unique_ptr<ISymmetricEncryption> symmetricEncryptor_;;
	std::shared_ptr<Logger> logger_;
};