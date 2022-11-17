#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

// https://learn.microsoft.com/ru-ru/windows/win32/winsock/winsock-server-application

#pragma comment(lib, "Ws2_32.lib")

constexpr const char* DEFAULT_PORT = "27015";
constexpr size_t DEFAULT_BUFLEN = 512;

int main()
{
    WSADATA wsaData;
    int code = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (code != 0)
    {
        std::cerr << "Error." << std::endl;
    }

    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    // Указываем семейство адресов IPv4
    hints.ai_family = AF_INET;
    // Тип сокета - SOCK_STREAM
    hints.ai_socktype = SOCK_STREAM;
    // Протокол TCP
    hints.ai_protocol = IPPROTO_TCP;
    // Флаг для указания того, что при bind мы будем использовать структуру адреса сокета
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    code = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (code != 0) {
        printf("getaddrinfo failed: %d\n", code);
        WSACleanup();
        return 1;
    }

    SOCKET listenSocket = INVALID_SOCKET;

    // Create a SOCKET for the server to listen for client 
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (listenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    code = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (code == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // no need anymore
    freeaddrinfo(result);

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket = INVALID_SOCKET;

    // Accept a client socket
    ClientSocket = accept(listenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    char recvbuf[DEFAULT_BUFLEN];
    int sendResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {

        code = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (code > 0) {
            printf("Bytes received: %d\n", code);

            // Echo the buffer back to the sender
            sendResult = send(ClientSocket, recvbuf, code, 0);
            if (sendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", sendResult);
        }
        else if (code == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (code > 0);

    // shutdown the send half of the connection since no more data will be sent
    code = shutdown(ClientSocket, SD_SEND);
    if (code == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}
