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
	Connection() : logger_(new CryptoPP::FileSink(std::cout))
	{
		socket_.MakeConnection();
	}

	void LogKey(const std::string& message, const std::string& key)
	{
		std::cout << message;
		logger_.Put((const byte*)&key[0], key.size());
		logger_.MessageEnd();
		std::cout << std::endl;
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

		SecByteBlock key(AES::MAX_KEYLENGTH);
		SecByteBlock iv(AES::BLOCKSIZE);

		rng.GenerateBlock(key, key.size());
		rng.GenerateBlock(iv, iv.size());

		// выведем на экран key и iv для проверки
		std::cout << "Generated AES session key\n";
		LogKey("key: ", { reinterpret_cast<const char*>(key.data()), key.size() });

		LogKey("iv: ", { reinterpret_cast<const char*>(iv.data()), iv.size() });

		// Шифрование сессионного ключа публичным
		std::cout << "Sending cipher AES session key to server..." << std::endl;

		std::string cipher_key, cipher_iv;

		// Шифруем AES сессионный ключ key
		try
		{
			RSAES_OAEP_SHA_Encryptor e(publicKey);

			ArraySource asKey(key, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_key)));
			
			// Шифруем AES сессионный ключ iv
			ArraySource asIv(iv, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_iv)));
		}
		catch (const Exception &e)
		{
			std::cerr << "AES session iv Encryption: " << e.what() << std::endl;
			exit(1);
		}

		// Выведем в консоль cipher_key для проверки
		LogKey("cipher_key: ", cipher_key);

		// Выведем в консоль cipher_iv для проверки
		LogKey("cipher_iv: ", cipher_iv);

		// Отправка на сервер сессионного ключа key
		socket_.Send({ &cipher_key[0], cipher_key.size() });

		// Отправка на сервер сессионного ключа iv
		socket_.Send({ &cipher_iv[0], cipher_iv.size() });

		while (true) {
			Sleep(30000);
		}
	}

	~Connection() {}

private:
	CryptoPP::HexEncoder logger_;
	ClientSocket socket_;
};