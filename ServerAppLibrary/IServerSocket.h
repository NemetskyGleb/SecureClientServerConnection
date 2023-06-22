#pragma once

#include <string>

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
