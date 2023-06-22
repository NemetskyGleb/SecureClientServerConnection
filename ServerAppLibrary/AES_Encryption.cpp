#include "AES_Encryption.h"

#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <cryptopp/aes.h>

using namespace CryptoPP; 

AES_Encryption::AES_Encryption(const SecByteBlock& key, const SecByteBlock& iv)
	: key_{ key }, iv_{ iv }
{
}


std::string AES_Encryption::Encrypt(const std::string& plainText)
{
	if (key_.empty())
	{
		throw std::runtime_error("key wasn't setted up");
	}
	if (iv_.empty())
	{
		throw std::runtime_error("iv wasn't setted up");
	}

	CBC_Mode<AES>::Encryption encryptor;

	std::string cipherText;

	encryptor.SetKeyWithIV(key_, key_.size(), iv_);
	StringSource sMessage(plainText, true,
		new StreamTransformationFilter(encryptor,
			new StringSink(cipherText)
		) // StreamTransformationFilter
	); // StringSource

	return cipherText;
}

std::string AES_Encryption::Decrypt(const std::string& cipherText)
{
	if (key_.empty())
	{
		throw std::runtime_error("key wasn't setted up");
	}
	if (iv_.empty())
	{
		throw std::runtime_error("iv wasn't setted up");
	}

	CBC_Mode<AES>::Decryption decryptor;

	decryptor.SetKeyWithIV(key_, key_.size(), iv_);

	std::string decryptedString;

	StringSource sSource(cipherText, true,
		new StreamTransformationFilter(decryptor,
			new StringSink(decryptedString)
		) // StreamTransformationFilter
	); // StringSource

	return decryptedString;
}

void AES_Encryption::SetSessionKey(const CryptoPP::SecByteBlock& key)
{
	key_ = key;
}

void AES_Encryption::SetKeyIv(const CryptoPP::SecByteBlock& iv)
{
	iv_ = iv;
}

size_t AES_Encryption::GetKeyBlockSize()
{
	return AES::MAX_KEYLENGTH;
}

size_t AES_Encryption::GetIvBlockSize()
{
	return AES::BLOCKSIZE;
}
