#include "pch.h"
#include "ServerSocket.h"
#include "ClientSocket.h"

class ServerSocketTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		// Start the server in a separate thread
		serverThread_ = std::thread([this]() {
			serverSocket_ = std::make_unique<ServerSocket>();
			serverSocket_->MakeConnection();
		});

		// Wait for the server to start
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	void TearDown() override
	{
		serverThread_.join();
	}

protected:
	// ServerSocket instance used in the tests
	std::unique_ptr<ServerSocket> serverSocket_;

	// Thread for running the server
	std::thread serverThread_;
};

TEST_F(ServerSocketTest, SendMessageAndRecieve)
{
	ClientSocket clientSocket;
	clientSocket.MakeConnection();

	std::string messageToSend = "Hello, Server!";
	clientSocket.Send(messageToSend);

	std::string receivedMessage = serverSocket_->WaitForRequest();

	EXPECT_EQ(receivedMessage, messageToSend);

	std::string response = "Hello, Client!";
	serverSocket_->Send(response);

	std::string receivedResponse = clientSocket.WaitForResponse();

	EXPECT_EQ(receivedResponse, response);
}
