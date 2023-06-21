#pragma once

#include <cryptopp/rsa.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include "ServerSocket.h"
#include "Logger.h"

#include <string>

/// @brief ����� ��� �������� ����������� ���������� � ��������, ��������� ���������� ���������
class SecureConnection
{
public:
	SecureConnection(std::unique_ptr<ServerSocket> socket, std::shared_ptr<Logger> logger);

	~SecureConnection();

	/// @brief ������� RSA ����������, � ������� ����� ��������� ���������� ���� ��� ��������� � ��� ����
	void MakeRsaConnection();

	/// @brief ��������� ������������� ��������� �������
	/// @param message ���������
	void SendSecuredMessage(const std::string& message);

	/// @brief �������� ��������� �� �������
	/// @return ���������� ���������
	std::string RecieveMessage();

private:
	CryptoPP::RSA::PrivateKey privateKey_;

	CryptoPP::SecByteBlock sessionKey_;
	CryptoPP::SecByteBlock iv_;

	CryptoPP::SecByteBlock sessionHashKey_;
	CryptoPP::SecByteBlock hashIv_;

	std::unique_ptr<ServerSocket> socket_;
	std::shared_ptr<Logger> logger_;

	CryptoPP::SHA256 hash_;
};