#pragma once

#include "IAsymmetricEncryption.h"
#include "ISymmetricEncryption.h"
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>

class RSAEncryption : public IAsymmetricEncryption
{

public:
	/// @brief ctor
	/// @param keySize ������ �����
	RSAEncryption(size_t keySize);

	void Encrypt(const std::string& plainText) const override;

	/// @brief ������������ ������������������ � ������� ���������� �����
	/// @param cipherText ������������� ������������������
	/// @param bufLength ����� ������ �������������� ������������������
	CryptoPP::SecByteBlock Decrypt(const std::string& cipherText, size_t bufLength) override;

	/// @brief ���������� �������������� ���� � DER �������
	std::string GetPublicKey() const override;
	
private:
	CryptoPP::RSA::PrivateKey privateKey_;
	CryptoPP::RSA::PublicKey publicKey_;

	CryptoPP::AutoSeededRandomPool rng_;
};