#pragma once

#include "IAsymmetricEncryption.h"
#include "ISymmetricEncryption.h"
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>

class RSAEncryption : public IAsymmetricEncryption
{

public:
	/// @brief ctor
	/// @param keySize Размер ключа
	RSAEncryption(size_t keySize);

	void Encrypt(const std::string& plainText) const override;

	/// @brief Расшифровать Последовательность с помощью приватного ключа
	/// @param cipherText Зашифрованная последовательность
	/// @param bufLength Длина буфера расшифрованной последовательности
	CryptoPP::SecByteBlock Decrypt(const std::string& cipherText, size_t bufLength) override;

	/// @brief Возвращает закодированный ключ в DER формате
	std::string GetPublicKey() const override;
	
private:
	CryptoPP::RSA::PrivateKey privateKey_;
	CryptoPP::RSA::PublicKey publicKey_;

	CryptoPP::AutoSeededRandomPool rng_;
};