#pragma once

#include "IAsymmetricEncryption.h"

class ISecureConnection
{
public:

	virtual ~ISecureConnection() = default;

	/// @brief ������� ���������� ����������, � ������� ����� ��������� ���������� ���� ��� ��������� � ��� ����
	/// @param provider ��������� �������������� ����������
	virtual void MakeSecureConnection(IAsymmetricEncryption* provider) = 0;

	/// @brief ��������� ������������� ��������� �������
	/// @param message ���������
	virtual void SendSecuredMessage(const std::string& message) = 0;

	/// @brief �������� ��������� �� �������
	/// @return ���������� ���������
	virtual std::string RecieveMessage() = 0;
};