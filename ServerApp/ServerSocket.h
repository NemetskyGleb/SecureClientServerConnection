#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>
#include <stdexcept>

/// @brief 
class ServerSocket
{
public:
	/// @brief ����������� ������
	/// @param port ���� ��� ������������ TCP ����������. �� ��������� 27015
	/// @param buflen ����� ������ ��� ��������� ���������. �� ��������� 512
	/// @param servername IP ����� �������. �� ��������� localhost
	ServerSocket(const std::string& port = "27015",
		size_t buflen = 512)
		: port_{ port }
	{
		recvbuf_.reserve(buflen);

		ZeroMemory(&hints_, sizeof(hints_));
		// IPv4 address
		hints_.ai_family = AF_INET;
		hints_.ai_socktype = SOCK_STREAM;
		hints_.ai_protocol = IPPROTO_TCP;
		hints_.ai_flags = AI_PASSIVE;

		// Initialize Winsock
		checkRetVal(WSAStartup(MAKEWORD(2, 2), &wsaData), "WSAStartup failed");
	}

	/// @brief ������� ����� ��� ����������� � ������� �� ��������� �����
	void MakeConnection();

	/// @brief ��������� �� ������ ���������
	/// @param message ������������ ���������
	void Send(const std::string& message);

	/// @brief ����� ��������� � �������
	/// @return ���������� ��������� 
	std::string WaitForRequest();

	~ServerSocket();

private:
	// ������ ����
	addrinfo* result_ = nullptr;

	// ��������� ��� ���� �������
	addrinfo hints_;

	// ����� ��� �������������
	SOCKET listenSocket_ = INVALID_SOCKET;
	// ����� ������ ����� ����������
	SOCKET clientSocket_ = INVALID_SOCKET;

	// ������ WinSock
	WSADATA wsaData;

	/// port ���� ��� ������������ TCP ����������. 
	std::string port_;

	// ����� ��� ��������� ���������
	std::string recvbuf_;

	// �������� ������������� ����, ����������� � ���������� ���������
	inline int32_t checkRetVal(int32_t statusCode, const char* errorMessage) {
		if (statusCode == SOCKET_ERROR)
		{
			if (result_)
			{
				freeaddrinfo(result_);
			}
			closesocket(listenSocket_);
			throw std::runtime_error(std::string(errorMessage) + ".Socket error.");
		}
		else if (statusCode != 0)
		{
			throw std::runtime_error(std::string(errorMessage) + " Status code = " + std::to_string(statusCode));
		}
		return statusCode;
	}
};