#include "Connection.h"
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

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
	// Accept client connection
	std::string recievedMessage = socket_.WaitForRequest();
	if (recievedMessage != "connection_start") {
		throw std::runtime_error("Failed connection!");
	}

	AutoSeededRandomPool rng;

	constexpr size_t keySize = 3072;

	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, keySize);
	privateKey_ = { params };
	RSA::PublicKey publicKey(params);

	//byte bytesBuf[keySize];
	std::string publicKeyStr;
	StringSink s(publicKeyStr);
	// Кодируем публичный ключ с помощью DER
	publicKey.Save(s);

	// Отправляем public key, private key сохраняем у себя
	socket_.Send({ &publicKeyStr[0], publicKeyStr.size() });

	// Получаем от клиента зашифрованный сессионный ключ
	std::cout << "Receiving cipher AES session key from client..." << std::endl;
	std::string sessionCipherKey = socket_.WaitForRequest();
	LogKey("cipher_key: ", sessionCipherKey);
	std::string sessionCipherIv = socket_.WaitForRequest();
	LogKey("cipher_iv: ", sessionCipherIv);

	// Получаем от клиента зашифрованный сессионный ключ для хеша
	std::cout << "Receiving cipher AES hash session key from client..." << std::endl;
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
	// Вычисление хеша от отправляемого сообщения
	std::string digest;
	StringSource sHash(message, true, new HashFilter(hash_, new StringSink(digest)));
	LogKey("Digest: ", digest);

	// Шифрование хеша и сообщения на сессионом ключе
	std::string cipherDigest;
	std::string cipherMessage;
	try
	{
		CBC_Mode<AES>::Encryption e;

		e.SetKeyWithIV(sessionKey_, sessionKey_.size(), iv_);
		StringSource sMessage(message, true,
			new StreamTransformationFilter(e,
				new StringSink(cipherMessage)
			) // StreamTransformationFilter
		); // StringSource

		e.SetKeyWithIV(sessionHashKey_, sessionHashKey_.size(), hashIv_);
		StringSource sHash(digest, true,
			new StreamTransformationFilter(e,
				new StringSink(cipherDigest)
			) // StreamTransformationFilter
		); // StringSource
	}
	catch (const Exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	socket_.Send({ &cipherMessage[0], cipherMessage.size() });
	LogKey("Cipher message: ", cipherMessage);
	Sleep(300);
	socket_.Send( { &cipherDigest[0], cipherDigest.size() } );
	LogKey("Cipher digest: ", cipherDigest);
}

std::string Connection::RecieveMessageFromClient()
{
	auto recievedCipherMessage = socket_.WaitForRequest();
	auto recievedCipherDigest = socket_.WaitForRequest();

	LogKey("Cipher message: ", recievedCipherMessage);
	LogKey("Cipher hash: ", recievedCipherDigest);

	std::string recievedMessage, recievedDigest;
	try
	{
		CBC_Mode<AES>::Decryption d;
		
		d.SetKeyWithIV(sessionHashKey_, sessionHashKey_.size(), hashIv_);

		StringSource sHash(recievedCipherDigest, true,
			new StreamTransformationFilter(d,
				new StringSink(recievedDigest)
			) // StreamTransformationFilter
		); // StringSource

		d.SetKeyWithIV(sessionKey_, sessionKey_.size(), iv_);

		StringSource sMessage(recievedCipherMessage, true,
			new StreamTransformationFilter(d,
				new StringSink(recievedMessage)
			) // StreamTransformationFilter
		); // StringSource
	}
	catch (const Exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	LogKey("Recieved hash: ", recievedDigest);

	std::string actualDigest;
	StringSource sHash(recievedMessage, true, new HashFilter(hash_, new StringSink(actualDigest)));

	LogKey("Actual hash: ", actualDigest);

	if (actualDigest != recievedDigest)
	{
		throw std::runtime_error("Hashes aren't equal. Authentification not passed.");
	}

	return recievedMessage;
}

