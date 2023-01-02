#include <iostream>

#include "ClientSocket.h"
#include "Connection.h"

int main(int argc, char** argv)
{
    try
    {        
        Connection connect;
        connect.RSAConnection();
         
        std::cout << "Everything is set up and ready to use!" << std::endl;
        const std::string message = "Client Hello!";
        connect.SendSecuredMessage(message);
        std::string recievedMessage = connect.RecieveMessageFromServer();
        std::cout << "Message from server: " << recievedMessage << std::endl;
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error" << std::endl;
    }

    return 0;
}