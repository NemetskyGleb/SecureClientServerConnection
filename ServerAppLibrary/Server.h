#pragma once

#include "SecureConnection.h"
#include "ServerSocket.h"
#include "Logger.h"


class Server
{
public:
	Server(IAsymmetricEncryption* provider, ISymmetricEncryption* symmetricProvider);

	~Server();

	void Start();

	SecureConnection& GetConnection();

private:
	std::shared_ptr<Logger> logger_;
	SecureConnection connection_;

	IAsymmetricEncryption* provider_;
};
