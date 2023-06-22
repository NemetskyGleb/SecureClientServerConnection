#include "Server.h"
#include "WindowsServerSocket.h"

Server::Server(const Settings& socketSettings,
			   IAsymmetricEncryption* provider,
			   ISymmetricEncryption* symmetricProvider)
	: logger_{ std::make_shared<Logger>() }
	, connection_{ std::make_unique<SecureConnection>(
				   std::make_unique<WindowsServerSocket>(socketSettings), symmetricProvider, logger_)}
	, provider_{ provider }
{	
}

Server::~Server()
{
}

void Server::Start()
{
	connection_->MakeSecureConnection(provider_);
}

ISecureConnection* Server::GetConnection()
{
	return connection_.get();
}
