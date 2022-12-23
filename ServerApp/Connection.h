#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

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
		// �������� ��������� ���� � ������� DER
		publicKey.Save(sink);

		// ���������� public key, private key ��������� � ����
		socket_.Send({ bytesBuf, bytesBuf + keySize });

		// �������� �� ������� ������������� ���������� ���� key
		std::cout << "Receiving cipher AES session key from client..." << endl;

		HexEncoder encoder(new FileSink(std::cout));

		std::string sessionCipherKey_key = socket_.WaitForRequest();

		// ������� � ������� ������������� ���������� ���� key, ������� ��� ������� �� �������
		std::cout << "cipher_key: ";
		encoder.Put((const byte *)&sessionCipherKey_key[0], sessionCipherKey_key.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		// �������� �� ������� ������������� ���������� ���� iv
		std::string sessionCipherKey_iv = socket_.WaitForRequest();

		// ������� � ������� ������������� ���������� ���� iv, ������� ��� ������� �� �������
		std::cout << "cipher_iv: ";
		encoder.Put((const byte *)&sessionCipherKey_iv[0], sessionCipherKey_iv.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		while (true) {
			Sleep(30000);
		}
	}



	~Connection() {}
private:
	ServerSocket socket_;

};