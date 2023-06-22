#pragma once

#include "SecureConnection.h"
#include "Logger.h"

class Server
{
public:
	Server(const Settings& socketSettings, 
		   IAsymmetricEncryption* provider,
		   ISymmetricEncryption* symmetricProvider);

	~Server();

	void Start();

	ISecureConnection* GetConnection();

private:
	std::shared_ptr<Logger> logger_;
	std::unique_ptr<ISecureConnection> connection_;

	IAsymmetricEncryption* provider_;
};
