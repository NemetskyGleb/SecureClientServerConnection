#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>

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
		using namespace std;

		// Accept client connection
		string recievedMessage = socket_.WaitForRequest();
		if (recievedMessage != "connection_start") {
			throw runtime_error("Failed connection!");
		}

		RSA::PublicKey pubKey;

		AutoSeededRandomPool rng;

		constexpr size_t keySize = 3072;

		InvertibleRSAFunction params;
		params.GenerateRandomWithKeySize(rng, keySize);

		RSA::PrivateKey privateKey(params);
		RSA::PublicKey publicKey(params);
		
		byte bytesBuf[keySize];
		ArraySink sink(bytesBuf, keySize);
		// Кодируем публичный ключ с помощью DER
		publicKey.Save(sink);

		// Отправляем public key, private key сохраняем у себя
		socket_.Send({ bytesBuf, bytesBuf + keySize });
	}



	~Connection() {}
private:
	ServerSocket socket_;

};