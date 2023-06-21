#include "Server.h"

Server::Server(IAsymmetricEncryption* provider)
	: logger_{ std::make_shared<Logger>() }
	, connection_{ std::make_unique<ServerSocket>(), logger_ }
	, provider_{ provider }
{	
}

Server::~Server()
{
}

void Server::Start()
{
	connection_.MakeSecureConnection(provider_);
}

SecureConnection& Server::GetConnection()
{
	return connection_;
}
