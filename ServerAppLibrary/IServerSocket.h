#pragma once

#include <string>

const std::string DEFAULT_PORT = "27015";
const size_t DEFAULT_BUFLEN = 512;

/// @brief ��������� ������
struct Settings
{
	std::string port = DEFAULT_PORT; /// ���� ��� ������������ TCP ����������. �� ��������� 27015
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
