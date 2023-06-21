#pragma once

#include <string>
#include <cryptopp/hex.h>

class Logger
{
public:
	Logger();

	/// @brief Вывести в консоль hex представление ключа
	/// @param message Сообщение
	/// @param key Ключ
	void LogKey(const std::string& message, const std::string& key);

	void Log(const std::string& message);

	~Logger();

private:
	CryptoPP::HexEncoder logger_;

	std::mutex mtx;
};