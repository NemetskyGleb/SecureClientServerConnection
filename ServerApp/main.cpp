#include <iostream>
#include <stdexcept>

#include "ServerSocket.h"
#include "ServerConnection.h"

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    try
    {
        ServerConnection connect;
        connect.RSAConnection();
        std::cout << "Everything is set up and ready to use!" << std::endl;
        std::cout << "Recieved message from client: " << connect.RecieveMessageFromClient() << std::endl;

        const std::string message = "Server Hello!";
        std::cout << "Sending \"" << message << "\" to client." << std::endl;
        connect.SendSecuredMessage(message);
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
