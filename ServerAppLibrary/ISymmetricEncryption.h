#pragma once

#include <string>

class ISymmetricEncryption
{
public:
	virtual ~ISymmetricEncryption() = default;

	virtual void Encrypt(const std::string& plainText) = 0;
	virtual void Decrypt(const std::string& cipherText) = 0;

	//setKey
	//setIV
};