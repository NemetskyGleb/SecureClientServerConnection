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

	sessionKey_ = SecByteBlock(AES::MAX_KEYLENGTH);
	iv_ = SecByteBlock(AES::BLOCKSIZE);
	
	sessionHashKey_ = SecByteBlock(AES::MAX_KEYLENGTH);
	hashIv_ = SecByteBlock(AES::BLOCKSIZE);

	rng.GenerateBlock(sessionKey_, sessionKey_.size());
	rng.GenerateBlock(iv_, iv_.size());

	rng.GenerateBlock(sessionHashKey_, sessionHashKey_.size());
	rng.GenerateBlock(hashIv_, hashIv_.size());

	std::cout << "Generated AES session key\n";
	LogKey("key: ", { reinterpret_cast<const char*>(sessionKey_.data()), sessionKey_.size() });
	LogKey("iv: ", { reinterpret_cast<const char*>(iv_.data()), iv_.size() });

	std::cout << "Generated AES session hash key\n";
	LogKey("hash key: ", { reinterpret_cast<const char*>(sessionHashKey_.data()), sessionHashKey_.size() });
	LogKey("hash iv: ", { reinterpret_cast<const char*>(hashIv_.data()), hashIv_.size() });

	// ���������� ����������� ����� ���������
	std::cout << "Sending cipher AES session keys to server..." << std::endl;

	std::string cipher_key, cipher_iv;
	std::string cipher_hash_key, cipher_hash_iv;


	// ������� AES ���������� �����
	try
	{
		RSAES_OAEP_SHA_Encryptor e(publicKey);

		ArraySource asKey(sessionKey_, sessionKey_.size(), true, /* pump all data */
			new PK_EncryptorFilter(rng, e,
				new StringSink(cipher_key)));

		ArraySource asHashKey(sessionHashKey_, sessionHashKey_.size(), true, /* pump all data */
			new PK_EncryptorFilter(rng, e,
				new StringSink(cipher_hash_key)));

		ArraySource asIv(iv_, iv_.size(), true, /* pump all data */
			new PK_EncryptorFilter(rng, e,
				new StringSink(cipher_iv)));

		ArraySource asHashIv(hashIv_, hashIv_.size(), true, /* pump all data */
			new PK_EncryptorFilter(rng, e,
				new StringSink(cipher_hash_iv)));
	}
	catch (const Exception& e)
	{
		std::cerr << "AES session iv Encryption: " << e.what() << std::endl;
		exit(1);
	}

	// �������� �� ������ ����������� ����� key
	socket_.Send({ &cipher_key[0], cipher_key.size() });
	// ������� � ������� cipher_key ��� ��������
	LogKey("cipher_key: ", cipher_key);
	// �������� �� ������ ����������� ����� iv
	socket_.Send({ &cipher_iv[0], cipher_iv.size() });
	// ������� � ������� cipher_iv ��� ��������
	LogKey("cipher_iv: ", cipher_iv);

	// �������� �� ������ ����������� ����� key
	socket_.Send({ &cipher_hash_key[0], cipher_hash_key.size() });
	// ������� � ������� cipher_hash_key ��� ��������
	LogKey("cipher_hash_key: ", cipher_hash_key);
	// �������� �� ������ ����������� ����� iv
	socket_.Send({ &cipher_hash_iv[0], cipher_hash_iv.size() });
	// ������� � ������� cipher_hash_iv ��� ��������
	LogKey("cipher_hash_iv: ", cipher_hash_iv);
}

void Connection::SendSecuredMessage(const std::string& message)
{
}

std::string Connection::RecieveMessageFromServer()
{
	return std::string();
}
