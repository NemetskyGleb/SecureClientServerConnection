#include <iostream>

#include "ServerSocket.h"

int main()
{
    try
    {
        ServerSocket socket;

        socket.MakeConnection();
        std::string recievedMessage = socket.WaitForRequest();
        std::cout << "Recieved message: " << recievedMessage << std::endl;

        socket.Send("Server Hello");
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
