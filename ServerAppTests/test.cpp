#include "pch.h"
#include "ServerApp.h"
#include "ClientApp.h"
#include <thread>

class ServerSocketTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// Start the server in a separate thread
		serverThread = std::thread([this]() {
			serverSocket = std::make_unique<ServerSocket>("27015", 512);
			serverSocket->MakeConnection();
			});

		// Wait for the server to start
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	void TearDown() override
	{
		// Stop the server and wait for the thread to join
		serverSocket->~ServerSocket();
		serverThread.join();
		serverSocket.reset();
	}

	// ServerSocket instance used in the tests
	std::unique_ptr<ServerSocket> serverSocket;

	// Thread for running the server
	std::thread serverThread;
};

TEST_F(ServerSocketTest, SendMessageAndRecieve)
{
	ClientSocket clientSocket;
	clientSocket.MakeConnection();

	std::string messageToSend = "Hello, Server!";
	clientSocket.Send(messageToSend);

	std::string receivedMessage = serverSocket->WaitForRequest();

	EXPECT_EQ(receivedMessage, messageToSend);

	std::string response = "Hello, Client!";
	serverSocket->Send(response);

	std::string receivedResponse = clientSocket.WaitForResponse();

	EXPECT_EQ(receivedResponse, response);
}
