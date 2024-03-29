#pragma once

#include "IServerSocket.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <list>
#include <vector>
#include <string>
#include <stdexcept>

/// @brief 
class WindowsServerSocket : public IServerSocket
{
public:
	/// @brief ����������� ������
	/// @param 
	/// @param servername IP ����� �������. �� ��������� localhost
	WindowsServerSocket(const Settings& settings);

	/// @brief ������� ����� ��� ����������� � ������� �� ��������� �����
	void MakeConnection() override;

	/// @brief ��������� �� ������ ���������
	/// @param message ������������ ���������
	void Send(const std::string& message) const override;

	/// @brief ����� ��������� � �������
	/// @return ���������� ��������� 
	std::string WaitForRequest() override;

	~WindowsServerSocket();

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

	// ��������� ������
	Settings settings_;

	// ����� ��� ��������� ���������
	std::string recvbuf_;

	// ��������� �� ������, ���������� ������������
	std::list<std::string> pendingMessages;

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