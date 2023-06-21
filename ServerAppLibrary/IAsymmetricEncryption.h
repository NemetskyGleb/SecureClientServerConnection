#pragma once

#include <string>

class IAsymmetricEncryption
{
public:
	virtual ~IAsymmetricEncryption() = default;
	
	virtual void SetPublicKey() = 0;
	virtual void SetPrivateKey() = 0;

	virtual void Encrypt(const std::string& plainText) = 0;
	virtual void Decrypt(const std::string& cipherText) = 0;
};