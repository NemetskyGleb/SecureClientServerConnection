#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>

/// @brief ����� ��� �������� ����������� ���������� �� ������� �������
class Connection
{
public:
	Connection();

	/// @brief ������� � ������� hex ������������� �����
	/// @param message ���������
	/// @param key ����
	void LogKey(const std::string& message, const std::string& key);

	/// @brief ������� RSA ����������, � ������� ����� ��������� ���������� ���� ��� ��������� � ��� ����
	void RSAConnection();

	/// @brief ��������� ������������� ��������� �������
	/// @param message ���������
	void SendSecuredMessage(const std::string& message);

	/// @brief �������� ��������� �� �������
	/// @return ���������� ���������
	std::string RecieveMessageFromClient();

	~Connection() {}
private:
	CryptoPP::RSA::PrivateKey privateKey_;
	
	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	ServerSocket socket_;
	CryptoPP::HexEncoder logger_;
};