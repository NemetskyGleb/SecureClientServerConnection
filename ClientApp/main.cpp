#include <iostream>

#include "ClientSocket.h"
#include "Connection.h"

int main(int argc, char** argv)
{
    try
    {
        Connection connect;
        connect.RSAConnection();
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