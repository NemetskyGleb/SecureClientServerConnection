#include <iostream>

#include "ClientSocket.h"

int main(int argc, char** argv)
{
    try
    {
        ClientSocket socket;

        socket.MakeConnection();

        socket.Send("Client Hello");

        std::string recievedMessage = socket.WaitForResponse();
        std::cout << "Recieved message: " << recievedMessage << std::endl;
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