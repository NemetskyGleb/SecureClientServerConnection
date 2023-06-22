#include "RSA_Encryption.h"

using namespace CryptoPP;

RSAEncryption::RSAEncryption(size_t keySize)
{
	InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng_, keySize);
	privateKey_ = { params };
	publicKey_ = { params };
}

void RSAEncryption::Encrypt(const std::string& plainText) const
{
	throw std::runtime_error("not implemented");
}


SecByteBlock RSAEncryption::Decrypt(const std::string& cipherText, size_t bufLength)
{
	SecByteBlock decryptedBlock(bufLength);

	RSAES_OAEP_SHA_Decryptor decryptor(privateKey_);

	StringSource stringSource(cipherText, true,
		new PK_DecryptorFilter(rng_, decryptor,
			new ArraySink(decryptedBlock, decryptedBlock.size())
		) // StreamTransformationFilter
	); // StringSource

	return decryptedBlock;
}

std::string RSAEncryption::GetPublicKey() const
{
	std::string publicKeyStr;
	StringSink stringSink(publicKeyStr);
	// Кодируем публичный ключ с помощью DER
	publicKey_.Save(stringSink);

	return publicKeyStr;
}
