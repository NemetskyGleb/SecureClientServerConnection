#include "ISymmetricEncryption.h"

class AESEncryption : public ISymmetricEncryption
{

public:
	void Encrypt(const std::string& plainText) override;

	void Decrypt(const std::string& cipherText) override;
private:

};