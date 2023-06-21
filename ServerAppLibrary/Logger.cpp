#include "Logger.h"

#include <cryptopp/files.h>

Logger::Logger() : logger_(new CryptoPP::FileSink(std::cout))
{
}

void Logger::LogKey(const std::string& message, const std::string& key)
{
	std::lock_guard<std::mutex> lock(mtx);

	std::cout << message;
	logger_.Put((const CryptoPP::byte*)&key[0], key.size());
	logger_.MessageEnd();
	std::cout << std::endl;
}


void Logger::Log(const std::string& message)
{
	std::lock_guard<std::mutex> lock(mtx);

	std::cout << message;
}

Logger::~Logger()
{
}
