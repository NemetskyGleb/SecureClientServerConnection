#pragma once

#include "IAsymmetricEncryption.h"


class RSAEncryption : public IAsymmetricEncryption
{

public:
	void Encrypt(const std::string& plainText) override;

	void Decrypt(const std::string& cipherText) override;

	void SetPublicKey() override;

	void SetPrivateKey() override;

private:

};