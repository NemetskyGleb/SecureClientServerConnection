#pragma once

#include <string>
#include <cryptopp/hex.h>

class Logger
{
public:
	Logger();

	/// @brief ������� � ������� hex ������������� �����
	/// @param message ���������
	/// @param key ����
	void LogKey(const std::string& message, const std::string& key);

	void Log(const std::string& message);

	~Logger();

private:
	CryptoPP::HexEncoder logger_;

	std::mutex mtx;
};