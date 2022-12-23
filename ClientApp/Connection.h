#pragma once
#include "ClientSocket.h"
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>
#include <string>

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
		AutoSeededRandomPool rng;
		HexEncoder encoder(new FileSink(std::cout));

		SecByteBlock key(AES::DEFAULT_KEYLENGTH);
		SecByteBlock iv(AES::BLOCKSIZE);

		rng.GenerateBlock(key, key.size());
		rng.GenerateBlock(iv, iv.size());

		// выведем на экран key и iv для проверки
		std::cout << "Generated AES session keys\nkey: ";
		encoder.Put(key, key.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		std::cout << "iv: ";
		encoder.Put(iv, iv.size());
		encoder.MessageEnd();
		std::cout << "\nSending AES session keys..." << std::endl;

		// Отправка на сервер
	}

	~Connection() {}

private:
	ClientSocket socket_;
};