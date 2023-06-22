#include "SecureConnection.h"

#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

SecureConnection::SecureConnection(std::unique_ptr<IServerSocket> socket,
								   ISymmetricEncryption* symmetricEncryptor,
								   std::shared_ptr<Logger> logger) 
	: socket_(std::move(socket))
	, symmetricEncryptor_{symmetricEncryptor}
	, logger_(logger)
{
	socket_->MakeConnection();
}

SecureConnection::~SecureConnection()
{
}

void SecureConnection::MakeSecureConnection(IAsymmetricEncryption* provider)
{
	// Accept client connection
	std::string recievedMessage = socket_->WaitForRequest();
	if (recievedMessage != "connection_start") {
		throw std::runtime_error("Failed connection!");
	}

	std::string publicKeyStr = provider->GetPublicKey();

	// Отправляем public key, private key сохраняем у себя
	socket_->Send({ &publicKeyStr[0], publicKeyStr.size() });

	// Получаем от клиента зашифрованный сессионный ключ
	logger_->Log("Receiving cipher AES session key from client...");
	std::string sessionCipherKey = socket_->WaitForRequest();
	logger_->LogKey("cipher_key: ", sessionCipherKey);
	std::string sessionCipherIv = socket_->WaitForRequest();
	logger_->LogKey("cipher_iv: ", sessionCipherIv);

	// Получаем от клиента зашифрованный сессионный ключ для хеша
	logger_->Log("Receiving cipher AES hash session key from client...");
	std::string sessionCipherHashKey = socket_->WaitForRequest();
	logger_->LogKey("cipher_hash_key: ", sessionCipherHashKey);
	std::string sessionCipherHashIv = socket_->WaitForRequest();
	logger_->LogKey("cipher_hash_iv: ", sessionCipherHashIv);

	try
	{
		// Расшифруем сессионные ключи полученные от клиента, используя приватный ключ
		sessionKey_ = provider->Decrypt(sessionCipherKey, symmetricEncryptor_->GetKeyBlockSize());
		iv_ = provider->Decrypt(sessionCipherIv, symmetricEncryptor_->GetIvBlockSize());

		sessionHashKey_ = provider->Decrypt(sessionCipherHashKey, symmetricEncryptor_->GetKeyBlockSize());
		hashIv_ = provider->Decrypt(sessionCipherHashIv, symmetricEncryptor_->GetIvBlockSize());
	}
	catch (const Exception& d)
	{
		std::cerr << "AES session key Decryption: " << d.what() << std::endl;
		exit(1);
	}

	logger_->Log( "Decrypted AES session key from client\n");
	logger_->LogKey("key: ", { reinterpret_cast<const char*>(sessionKey_.data()), sessionKey_.size() });
	logger_->LogKey("iv: ", { reinterpret_cast<const char*>(iv_.data()), iv_.size() });

	logger_->Log( "Decrypted AES hash session key from client\n");
	logger_->LogKey("hash_key: ", { reinterpret_cast<const char*>(sessionHashKey_.data()), sessionKey_.size() });
	logger_->LogKey("hash_iv: ", { reinterpret_cast<const char*>(hashIv_.data()), hashIv_.size() });
}

void SecureConnection::SendSecuredMessage(const std::string& message)
{
	// Вычисление хеша от отправляемого сообщения
	std::string digest;
	StringSource sHash(message, true, new HashFilter(hash_, new StringSink(digest)));
	logger_->LogKey("Digest: ", digest);

	// Шифрование хеша и сообщения на сессионом ключе
	std::string cipherDigest;
	std::string cipherMessage;
	try
	{
		symmetricEncryptor_->SetSessionKey(sessionKey_);
		symmetricEncryptor_->SetKeyIv(iv_);

		cipherMessage = symmetricEncryptor_->Encrypt(message);

		symmetricEncryptor_->SetSessionKey(sessionHashKey_);
		symmetricEncryptor_->SetKeyIv(hashIv_);

		cipherDigest = symmetricEncryptor_->Encrypt(digest);
	}
	catch (const Exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	socket_->Send({ &cipherMessage[0], cipherMessage.size() });
	logger_->LogKey("Cipher message: ", cipherMessage);
	socket_->Send({ &cipherDigest[0], cipherDigest.size() });
	logger_->LogKey("Cipher digest: ", cipherDigest);
}

std::string SecureConnection::RecieveMessage()
{
	auto recievedCipherMessage = socket_->WaitForRequest();
	auto recievedCipherDigest = socket_->WaitForRequest();

	logger_->LogKey("Cipher message: ", recievedCipherMessage);
	logger_->LogKey("Cipher hash: ", recievedCipherDigest);

	std::string recievedMessage, recievedDigest;
	try
	{

		symmetricEncryptor_->SetSessionKey(sessionKey_);
		symmetricEncryptor_->SetKeyIv(iv_);

		recievedMessage = symmetricEncryptor_->Decrypt(recievedCipherMessage);

		symmetricEncryptor_->SetSessionKey(sessionHashKey_);
		symmetricEncryptor_->SetKeyIv(hashIv_);

		recievedDigest = symmetricEncryptor_->Decrypt(recievedCipherDigest);
	}
	catch (const Exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	logger_->LogKey("Recieved hash: ", recievedDigest);

	std::string actualDigest;
	StringSource sHash(recievedMessage, true, new HashFilter(hash_, new StringSink(actualDigest)));

	logger_->LogKey("Actual hash: ", actualDigest);

	if (actualDigest != recievedDigest)
	{
		throw std::runtime_error("Hashes aren't equal. Authentification not passed.");
	}

	return recievedMessage;
}
