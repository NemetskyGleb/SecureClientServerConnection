#include "Connection.h"
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>


Connection::Connection() : logger_(new CryptoPP::FileSink(std::cout))
{
	socket_.MakeConnection();
}

void Connection::LogKey(const std::string& message, const std::string& key)
{
	std::cout << message;
	logger_.Put((const byte*)&key[0], key.size());
	logger_.MessageEnd();
	std::cout << std::endl;
}

void Connection::RSAConnection()
{
	using namespace CryptoPP;
	using namespace std;

	// Accept client connection
	string recievedMessage = socket_.WaitForRequest();
	if (recievedMessage != "connection_start") {
		throw runtime_error("Failed connection!");
	}

	AutoSeededRandomPool rng;

	constexpr size_t keySize = 3072;

	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, keySize);
	privateKey_ = { params };
	RSA::PublicKey publicKey(params);

	byte bytesBuf[keySize];
	ArraySink sink(bytesBuf, keySize);
	// �������� ��������� ���� � ������� DER
	publicKey.Save(sink);

	// ���������� public key, private key ��������� � ����
	socket_.Send({ bytesBuf, bytesBuf + keySize });

	// �������� �� ������� ������������� ���������� ���� key
	std::cout << "Receiving cipher AES session key from client..." << endl;

	std::string sessionCipherKey = socket_.WaitForRequest();
	// ������� � ������� ������������� ���������� ���� key, ������� ��� ������� �� �������
	LogKey("cipher_key: ", sessionCipherKey);

	// �������� �� ������� ������������� ���������� ���� iv
	std::string sessionCipherIv = socket_.WaitForRequest();
	// ������� � ������� ������������� ���������������� ������ iv, ������� ��� ������� �� �������
	LogKey("cipher_iv: ", sessionCipherIv);

	// ���������� ���������� ���� ���������� �� ������� ��������� ��������� ����
	sessionKey_ = SecByteBlock(AES::MAX_KEYLENGTH);
	iv_ = SecByteBlock(AES::BLOCKSIZE);

	try
	{
		RSAES_OAEP_SHA_Decryptor d(privateKey_);
		StringSource sKey(sessionCipherKey, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(sessionKey_, sessionKey_.size())
			) // StreamTransformationFilter
		); // StringSource

		StringSource sIv(sessionCipherIv, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(iv_, iv_.size())
			) // StreamTransformationFilter
		); // StringSource
	}
	catch (const Exception& d)
	{
		std::cerr << "AES session key Decryption: " << d.what() << std::endl;
		exit(1);
	}

	// ������� � ������� �������������� ���������� ���� key, ������� ��� ������� �� �������
	std::cout << "Decrypted AES session key from client\n";
	LogKey("key: ", { reinterpret_cast<const char*>(sessionKey_.data()), sessionKey_.size() });

	// ������� � ������� �������������� ���������������� ������ iv, ������� ��� ������� �� �������
	LogKey("iv: ", { reinterpret_cast<const char*>(iv_.data()), iv_.size() });
}

void Connection::SendSecuredMessage(const std::string& message)
{
}

std::string Connection::RecieveMessageFromClient()
{
	return std::string();
}

