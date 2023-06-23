#include "HashCalculator.h"

#include <cryptopp/filters.h>


std::string HashCalculator::CalculateHash(const std::string& message)
{
    std::string digest;
    CryptoPP::StringSource sHash(message, true, new CryptoPP::HashFilter(hash_, new CryptoPP::StringSink(digest)));
    return digest;
}

void HashCalculator::Verify(const std::string& message, const std::string& digest)
{

	std::string actualDigest;
	CryptoPP::StringSource sHash(message, true, new CryptoPP::HashFilter(hash_, new CryptoPP::StringSink(actualDigest)));

	if (actualDigest != digest)
	{
		throw std::runtime_error("Hashes aren't equal. Authentification not passed.");
	}
}
