#include "pch.h"
#include "ClientConnection.h"
#include "ServerConnection.h"

class SecuredConnectionTest : public ::testing::Test
{
protected:
	void SetUp() override
    {
        serverThread_ = std::thread([this]() {
            server_ = std::make_unique<ServerConnection>();
            server_->RSAConnection();

            std::unique_lock<std::mutex> lock(mutex_);
            serverReady_ = true;
            cv_.notify_all();
            lock.unlock();
        });
	}

    void TearDown() override
    {
        serverThread_.join();
    }

protected:
    std::unique_ptr<ServerConnection> server_;

    std::mutex mutex_;
    std::thread serverThread_;

    std::condition_variable cv_;
    bool serverReady_ = false;
};

// Test sending and receiving messages between client and server
TEST_F(SecuredConnectionTest, SendAndReceiveMessages) 
{
    ClientConnection client;
    client.RSAConnection();

    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return serverReady_; });
    lock.unlock();

    std::string message = "Hello, server!";

    ASSERT_NO_THROW(client.SendSecuredMessage(message));

    std::string receivedMessage = server_->RecieveMessageFromClient();

    ASSERT_NE(0, receivedMessage.size());

    ASSERT_EQ(receivedMessage, message);

    std::string response = "Hello, client!";
    server_->SendSecuredMessage(response);

    std::string receivedResponse = client.RecieveMessageFromServer();

    EXPECT_EQ(receivedResponse, response);
}
