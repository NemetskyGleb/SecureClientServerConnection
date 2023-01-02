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
	// Кодируем публичный ключ с помощью DER
	publicKey.Save(sink);

	// Отправляем public key, private key сохраняем у себя
	socket_.Send({ bytesBuf, bytesBuf + keySize });

	// Получаем от клиента зашифрованный сессионный ключ
	std::cout << "Receiving cipher AES session key from client..." << endl;
	std::string sessionCipherKey = socket_.WaitForRequest();
	LogKey("cipher_key: ", sessionCipherKey);
	std::string sessionCipherIv = socket_.WaitForRequest();
	LogKey("cipher_iv: ", sessionCipherIv);

	// Получаем от клиента зашифрованный сессионный ключ для хеша
	std::cout << "Receiving cipher AES hash session key from client..." << endl;
	std::string sessionCipherHashKey = socket_.WaitForRequest();
	LogKey("cipher_hash_key: ", sessionCipherHashKey);
	std::string sessionCipherHashIv = socket_.WaitForRequest();
	LogKey("cipher_hash_iv: ", sessionCipherHashIv);

	// Расшифруем сессионные ключи полученные от клиента, используя приватный ключ
	sessionKey_ = SecByteBlock(AES::MAX_KEYLENGTH);
	iv_ = SecByteBlock(AES::BLOCKSIZE);

	sessionHashKey_ = SecByteBlock(AES::MAX_KEYLENGTH);
	hashIv_ = SecByteBlock(AES::BLOCKSIZE);

	try
	{
		RSAES_OAEP_SHA_Decryptor d(privateKey_);

		StringSource sKey(sessionCipherKey, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(sessionKey_, sessionKey_.size())
			) // StreamTransformationFilter
		); // StringSource

		StringSource sHashKey(sessionCipherHashKey, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(sessionHashKey_, sessionHashKey_.size())
			) // StreamTransformationFilter
		); // StringSource

		StringSource sIv(sessionCipherIv, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(iv_, iv_.size())
			) // StreamTransformationFilter
		); // StringSource

		StringSource sHashIv(sessionCipherHashIv, true,
			new PK_DecryptorFilter(rng, d,
				new ArraySink(hashIv_, hashIv_.size())
			) // StreamTransformationFilter
		); // StringSource
	}
	catch (const Exception& d)
	{
		std::cerr << "AES session key Decryption: " << d.what() << std::endl;
		exit(1);
	}

	std::cout << "Decrypted AES session key from client\n";
	LogKey("key: ", { reinterpret_cast<const char*>(sessionKey_.data()), sessionKey_.size() });
	LogKey("iv: ", { reinterpret_cast<const char*>(iv_.data()), iv_.size() });

	std::cout << "Decrypted AES hash session key from client\n";
	LogKey("hash_key: ", { reinterpret_cast<const char*>(sessionHashKey_.data()), sessionKey_.size() });
	LogKey("hash_iv: ", { reinterpret_cast<const char*>(hashIv_.data()), hashIv_.size() });
}

void Connection::SendSecuredMessage(const std::string& message)
{
}

std::string Connection::RecieveMessageFromClient()
{
	return std::string();
}

