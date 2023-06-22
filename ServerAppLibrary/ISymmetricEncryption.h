#pragma once

#include <string>
#include <cryptopp/secblock.h>

class ISymmetricEncryption
{
public:
	virtual ~ISymmetricEncryption() = default;

	virtual std::string Encrypt(const std::string& plainText) = 0;
	virtual std::string Decrypt(const std::string& cipherText) = 0;

	virtual void SetSessionKey(const CryptoPP::SecByteBlock& key) = 0;
	virtual void SetKeyIv(const CryptoPP::SecByteBlock& iv) = 0;

	virtual size_t GetKeyBlockSize() = 0;
	virtual size_t GetIvBlockSize() = 0;
};