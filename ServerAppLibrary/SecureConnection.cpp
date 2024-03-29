#include "SecureConnection.h"
#include "HashCalculator.h"

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

	// ���������� public key, private key ��������� � ����
	socket_->Send({ &publicKeyStr[0], publicKeyStr.size() });

	// �������� �� ������� ������������� ���������� ����
	logger_->Log("Receiving cipher AES session key from client...");
	std::string sessionCipherKey = socket_->WaitForRequest();
	logger_->LogKey("cipher_key: ", sessionCipherKey);
	std::string sessionCipherIv = socket_->WaitForRequest();
	logger_->LogKey("cipher_iv: ", sessionCipherIv);

	// �������� �� ������� ������������� ���������� ���� ��� ����
	logger_->Log("Receiving cipher AES hash session key from client...");
	std::string sessionCipherHashKey = socket_->WaitForRequest();
	logger_->LogKey("cipher_hash_key: ", sessionCipherHashKey);
	std::string sessionCipherHashIv = socket_->WaitForRequest();
	logger_->LogKey("cipher_hash_iv: ", sessionCipherHashIv);

	try
	{
		// ���������� ���������� ����� ���������� �� �������, ��������� ��������� ����
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
	// ���������� ���� �� ������������� ���������
	HashCalculator hasher;
	std::string digest = hasher.CalculateHash(message);

	logger_->LogKey("Digest: ", digest);

	// ���������� ���� � ��������� �� ��������� �����
	std::string cipherDigest;
	std::string cipherMessage;
	try
	{
		cipherMessage = EncryptOnSessionKey(message, sessionKey_, iv_);
		cipherDigest = EncryptOnSessionKey(digest, sessionHashKey_, hashIv_);
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
		recievedMessage = DecryptWithSessionKey(recievedCipherMessage, sessionKey_, iv_);
		recievedDigest = DecryptWithSessionKey(recievedCipherDigest, sessionHashKey_, hashIv_);
	}
	catch (const Exception& e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}

	HashCalculator hasher;
	hasher.Verify(recievedMessage, recievedDigest);
	
	return recievedMessage;
}

std::string SecureConnection::EncryptOnSessionKey(const std::string& message, 
												  const CryptoPP::SecByteBlock& key,
												  const CryptoPP::SecByteBlock& iv)
{
	symmetricEncryptor_->SetSessionKey(key);
	symmetricEncryptor_->SetKeyIv(iv);

	return symmetricEncryptor_->Encrypt(message);
}

std::string SecureConnection::DecryptWithSessionKey(const std::string& message,
													const CryptoPP::SecByteBlock& key,
												    const CryptoPP::SecByteBlock& iv)
{
	symmetricEncryptor_->SetSessionKey(key);
	symmetricEncryptor_->SetKeyIv(iv);

	return symmetricEncryptor_->Decrypt(message);
}
