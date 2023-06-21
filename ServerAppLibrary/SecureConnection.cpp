#include "SecureConnection.h"

#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>

using namespace CryptoPP;

SecureConnection::SecureConnection(std::unique_ptr<ServerSocket> socket, std::shared_ptr<Logger> logger) 
	: socket_( std::move(socket) ), logger_(logger)
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
		sessionKey_ = provider->Decrypt(sessionCipherKey, AES::MAX_KEYLENGTH);
		iv_ = provider->Decrypt(sessionCipherIv, AES::BLOCKSIZE);

		sessionHashKey_ = provider->Decrypt(sessionCipherHashKey, AES::MAX_KEYLENGTH);
		hashIv_ = provider->Decrypt(sessionCipherHashIv, AES::BLOCKSIZE);
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
	std::string digest;
	StringSource sHash(message, true, new HashFilter(hash_, new StringSink(digest)));
	logger_->LogKey("Digest: ", digest);

	// ���������� ���� � ��������� �� ��������� �����
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
