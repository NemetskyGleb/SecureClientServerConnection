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

		SecByteBlock key(AES::MAX_KEYLENGTH);
		SecByteBlock iv(AES::BLOCKSIZE);

		rng.GenerateBlock(key, key.size());
		rng.GenerateBlock(iv, iv.size());

		// ������� �� ����� key � iv ��� ��������
		std::cout << "Generated AES session key\n";
		LogKey("key: ", { reinterpret_cast<const char*>(key.data()), key.size() });

		LogKey("iv: ", { reinterpret_cast<const char*>(iv.data()), iv.size() });

		// ���������� ����������� ����� ���������
		std::cout << "Sending cipher AES session key to server..." << std::endl;

		std::string cipher_key, cipher_iv;

		// ������� AES ���������� ���� key
		try
		{
			RSAES_OAEP_SHA_Encryptor e(publicKey);

			ArraySource asKey(key, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_key)));
			
			// ������� AES ���������� ���� iv
			ArraySource asIv(iv, key.size(), true, /* pump all data */
				new PK_EncryptorFilter(rng, e,
					new StringSink(cipher_iv)));
		}
		catch (const Exception &e)
		{
			std::cerr << "AES session iv Encryption: " << e.what() << std::endl;
			exit(1);
		}

		// ������� � ������� cipher_key ��� ��������
		LogKey("cipher_key: ", cipher_key);

		// ������� � ������� cipher_iv ��� ��������
		LogKey("cipher_iv: ", cipher_iv);

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
	CryptoPP::HexEncoder logger_;
	ClientSocket socket_;
};