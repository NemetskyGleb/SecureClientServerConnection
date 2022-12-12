#pragma once
#include "ClientSocket.h"
#include <cryptopp/rsa.h>

class Connection
{
public:
	Connection() 
	{
		socket_.MakeConnection();
	}
	void RSAConnection()
	{
		using namespace CryptoPP;
		// �������� RSA ���������� � ��������
		socket_.Send("connection_start");
		
		// �������� �� ������� ��������� ���� � DER ���������
		std::string publicKeyEncoded = socket_.WaitForResponse();

		ByteQueue bytes;
		bytes.Put(reinterpret_cast<byte*>(&publicKeyEncoded[0]), publicKeyEncoded.size());

		// ��������� � ������ ��������� ���� BER ��������������
		RSA::PublicKey publicKey;
		publicKey.Load(bytes);

		// �������� ���������� �����

		// ���������� ���������� ����� ����������
		// �������� �� ������
	}

	~Connection() {}

private:
	ClientSocket socket_;
};