#include "Server.h"

Server::Server()
	: logger_{ std::make_shared<Logger>() },
	connection_{ std::make_unique<ServerSocket>(), logger_ }
{	
}

Server::~Server()
{
}

void Server::Start()
{
	connection_.MakeRsaConnection();
}

SecureConnection& Server::GetConnection()
{
	return connection_;
}
