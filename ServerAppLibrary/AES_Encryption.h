#include "ISymmetricEncryption.h"

class AES_Encryption : public ISymmetricEncryption
{
public:
	AES_Encryption() = default;
	AES_Encryption(const CryptoPP::SecByteBlock& key, const CryptoPP::SecByteBlock& iv);

	virtual ~AES_Encryption() = default;

	std::string Encrypt(const std::string& plainText) override;
	std::string Decrypt(const std::string& cipherText) override;

	void SetSessionKey(const CryptoPP::SecByteBlock& key) override;
	void SetKeyIv(const CryptoPP::SecByteBlock& iv) override;

	size_t GetKeyBlockSize() override;
	size_t GetIvBlockSize() override;

private:
	CryptoPP::SecByteBlock key_;
	CryptoPP::SecByteBlock iv_;
};