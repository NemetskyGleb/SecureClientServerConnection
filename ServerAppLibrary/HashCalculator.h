#pragma once
#include <cryptopp/sha.h>

class HashCalculator 
{
public:
    std::string CalculateHash(const std::string& message);

    void Verify(const std::string& message, const std::string& digest);

private:
    CryptoPP::SHA256 hash_;
};