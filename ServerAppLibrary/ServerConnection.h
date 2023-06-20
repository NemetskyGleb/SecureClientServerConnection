#pragma once

#include "ServerSocket.h"
#include <iostream>
#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

/// @brief ����� ��� �������� ����������� ���������� �� ������� �������
class ServerConnection
{
public:
	ServerConnection();

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

	const ServerSocket& GetSocket() { return socket_;  }

	~ServerConnection() {}
private:
	CryptoPP::RSA::PrivateKey privateKey_;
	
	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	ServerSocket socket_;
	CryptoPP::SHA256 hash_;
	CryptoPP::HexEncoder logger_;
};