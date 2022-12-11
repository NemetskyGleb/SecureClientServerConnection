#pragma once
#include "ClientSocket.h"

class Connection
{
public:
	Connection() 
	{
		socket_.MakeConnection();
	}
	void RSAConnection()
	{
		socket_.Send("connection_start");
	}

	~Connection() {}

private:
	ClientSocket socket_;
};