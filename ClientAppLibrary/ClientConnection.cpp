#include "ClientConnection.h"
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/modes.h>

#include <mutex>
#include <thread>

using namespace CryptoPP;

std::mutex clientMtx;

ClientConnection::ClientConnection() : logger_(new FileSink(std::cout))
{
	socket_.MakeConnection();
}

void ClientConnection::LogKey(const std::string& message, const std::string& key)
{
	std::lock_guard<std::mutex> lock(clientMtx);

	std::cout << message;
	logger_.Put((const byte*)&key[0], key.size());
	logger_.MessageEnd();
	std::cout << std::endl;
}

void ClientConnection::RSAConnection()
{
	// Начинаем RSA соединение с сервером
	socket_.Send("connection_start");

	// Получаем от сервера публичный ключ в DER кодировке
	std::string publicKeyEncoded = socket_.WaitForResponse();

	StringSource ss(publicKeyEncoded, true);
	
	// Загружаем в клиент публичный ключ BER декодированием
	RSA::PublicKey publicKey;
	publicKey.Load(ss);

	// Создание сессионого ключа
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

	// Шифрование сессионного ключа публичным
	std::cout << "Sending cipher AES session keys to server..." << std::endl;

	std::string cipher_key, cipher_iv;
	std::string cipher_hash_key, cipher_hash_iv;


	// Шифруем AES сессионные ключи
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

	// Отправка на сервер сессионного ключа key
	socket_.Send({ &cipher_key[0], cipher_key.size() });
	// Выведем в консоль cipher_key для проверки
	LogKey("cipher_key: ", cipher_key);
	// Отправка на сервер сессионного ключа iv
	socket_.Send({ &cipher_iv[0], cipher_iv.size() });
	// Выведем в консоль cipher_iv для проверки
	LogKey("cipher_iv: ", cipher_iv);

	// Отправка на сервер сессионного ключа key
	socket_.Send({ &cipher_hash_key[0], cipher_hash_key.size() });
	// Выведем в консоль cipher_hash_key для проверки
	LogKey("cipher_hash_key: ", cipher_hash_key);
	// Отправка на сервер сессионного ключа iv
	socket_.Send({ &cipher_hash_iv[0], cipher_hash_iv.size() });
	// Выведем в консоль cipher_hash_iv для проверки
	LogKey("cipher_hash_iv: ", cipher_hash_iv);
}

void ClientConnection::SendSecuredMessage(const std::string& message)
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
	socket_.Send({ &cipherDigest[0], cipherDigest.size() });
	LogKey("Cipher digest: ", cipherDigest);
}

std::string ClientConnection::RecieveMessageFromServer()
{
	auto recievedCipherMessage = socket_.WaitForResponse();
	LogKey("Cipher message: ", recievedCipherMessage);

	auto recievedCipherDigest = socket_.WaitForResponse();
	LogKey("Cipher digest: ", recievedCipherDigest);

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
