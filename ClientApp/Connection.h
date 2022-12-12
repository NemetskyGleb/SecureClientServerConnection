#pragma once
#include "ClientSocket.h"
#include <cryptopp/rsa.h>

class Connection
{
public:
	Connection() 
	{
		socket_.MakeConnection();
	}
	void RSAConnection()
	{
		using namespace CryptoPP;
		// Начинаем RSA соединение с сервером
		socket_.Send("connection_start");
		
		// Получаем от сервера публичный ключ в DER кодировке
		std::string publicKeyEncoded = socket_.WaitForResponse();

		ByteQueue bytes;
		bytes.Put(reinterpret_cast<byte*>(&publicKeyEncoded[0]), publicKeyEncoded.size());

		// Загружаем в клиент публичный ключ BER декодированием
		RSA::PublicKey publicKey;
		publicKey.Load(bytes);

		// Создание сессионого ключа

		// Шифрование публичного ключа сессионным
		// Отправка на сервер
	}

	~Connection() {}

private:
	ClientSocket socket_;
};