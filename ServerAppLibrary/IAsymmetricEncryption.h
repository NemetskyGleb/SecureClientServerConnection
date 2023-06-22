#pragma once

#include <string>
#include <cryptopp/secblock.h>

class IAsymmetricEncryption
{
public:
	virtual ~IAsymmetricEncryption() = default;
	
	virtual std::string GetPublicKey() const = 0;

	virtual void Encrypt(const std::string& plainText) const = 0;
	virtual CryptoPP::SecByteBlock Decrypt(const std::string& cipherText, size_t bufLength) = 0;
};