#pragma once

#include <string>

/// @brief ��������� ������
struct Settings
{
	std::string port = "27015"; /// ���� ��� ������������ TCP ����������. �� ��������� 27015
	size_t buflen = 512; // ����� ������ ��� ��������� ���������. �� ��������� 512
};

class IServerSocket
{
public:
	virtual ~IServerSocket() = default;
	
	/// @brief ������� ����� ��� ����������� � ������� �� ��������� �����
	virtual void MakeConnection() = 0;

	/// @brief ��������� �� ������ ���������
	/// @param message ������������ ���������
	virtual void Send(const std::string& message) const = 0;

	/// @brief ����� ��������� � �������
	/// @return ���������� ��������� 
	virtual std::string WaitForRequest() = 0;
};
