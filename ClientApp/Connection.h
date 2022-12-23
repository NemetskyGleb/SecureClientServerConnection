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
		std::cout << std::endl;

		// ���������� ����������� ����� ���������
		std::cout << "Sending cipher AES session keys to server..." << std::endl;

		std::string cipher_key, cipher_iv;

		// ������� AES ���������� ���� key
		try
		{
			RSAES_OAEP_SHA_Encryptor e(publicKey);

			ArraySource as(key, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_key)));
		}
		catch (const Exception &e)
		{
			std::cerr << "AES session key Encryption: " << e.what() << std::endl;
			exit(1);
		}

		// ������� � ������� cipher_key ��� ��������
		std::cout << "cipher_key: ";
		encoder.Put((const byte *)&cipher_key[0], cipher_key.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		// ������� AES ���������� ���� iv
		try
		{
			RSAES_OAEP_SHA_Encryptor e(publicKey);

			ArraySource as(iv, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_iv)));
		}
		catch (const Exception &e)
		{
			std::cerr << "AES session iv Encryption: " << e.what() << std::endl;
			exit(1);
		}

		// ������� � ������� cipher_iv ��� ��������
		std::cout << "cipher_iv: ";
		encoder.Put((const byte *)&cipher_iv[0], cipher_iv.size());
		encoder.MessageEnd();
		std::cout << std::endl;

		// �������� �� ������ ����������� ����� key
		socket_.Send({ &cipher_key[0], cipher_key.size() });

		// �������� �� ������ ����������� ����� iv
		socket_.Send({ &cipher_iv[0], cipher_iv.size() });

		while (true) {
			Sleep(30000);
		}
	}

	~Connection() {}

private:
	ClientSocket socket_;
};