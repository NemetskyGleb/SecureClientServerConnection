#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/randpool.h>
#include <cryptopp/pubkey.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rng.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>

class Connection
{
public:
	Connection()
	{
		socket_.MakeConnection();
	}

	void RSAConnection() 
	{
		using namespace CryptoPP;
		using namespace std;

		// Accept client connection
		string recievedMessage = socket_.WaitForRequest();
		if (recievedMessage != "connection_start") {
			throw runtime_error("Failed connection!");
		}

		//RSA::PublicKey pubKey;

		AutoSeededRandomPool rng;

		InvertibleRSAFunction params;
		params.GenerateRandomWithKeySize(rng, 3072);

		RSA::PrivateKey privateKey(params);
		RSA::PublicKey publicKey(params);

		RSAES_OAEP_SHA_Encryptor e(publicKey);
		
		//char buf[256];
		//ByteQueue bytes;
		publicKey.BERDecode(FileSource("privkey.der").Ref());;
		//bytes.Put(reinterpret_cast<const byte*>(buf), 256);
	}

	

	~Connection() {}
private:
	ServerSocket socket_;

};