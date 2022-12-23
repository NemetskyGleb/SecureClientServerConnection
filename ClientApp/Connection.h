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

		// �������� RSA ���������� � ��������
		socket_.Send("connection_start");
		
		// �������� �� ������� ��������� ���� � DER ���������
		std::string publicKeyEncoded = socket_.WaitForResponse();

		ByteQueue bytes;
		bytes.Put(reinterpret_cast<byte*>(&publicKeyEncoded[0]), publicKeyEncoded.size());

		// ��������� � ������ ��������� ���� BER ��������������
		RSA::PublicKey publicKey;
		publicKey.Load(bytes);

		// �������� ���������� �����
		AutoSeededRandomPool rng;
		HexEncoder encoder(new FileSink(std::cout));

		SecByteBlock key(AES::DEFAULT_KEYLENGTH);
		SecByteBlock iv(AES::BLOCKSIZE);

		rng.GenerateBlock(key, key.size());
		rng.GenerateBlock(iv, iv.size());

		// ������� �� ����� key � iv ��� ��������
		std::cout << "Generated AES session keys\nkey: ";
		encoder.Put(key, key.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		std::cout << "iv: ";
		encoder.Put(iv, iv.size());
		encoder.MessageEnd();
		std::cout << "\nSending AES session keys..." << std::endl;

		// �������� �� ������
	}

	~Connection() {}

private:
	ClientSocket socket_;
};