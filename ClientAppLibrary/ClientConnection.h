#pragma once
#include "ClientSocket.h"
#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <string>

/// @brief ����� ��� �������� ����������� ���������� �� ������� �������
class ClientConnection
{
public:
	ClientConnection();

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
	std::string RecieveMessageFromServer();

	~ClientConnection() {}

private:
	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	CryptoPP::HexEncoder logger_;
	CryptoPP::SHA256 hash_;
	ClientSocket socket_;
};