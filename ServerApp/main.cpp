#include <iostream>
#include <stdexcept>

#include "ServerSocket.h"
#include "Connection.h"
#include <tchar.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
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
