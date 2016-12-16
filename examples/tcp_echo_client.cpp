/*!
    \file tcp_echo_client.cpp
    \brief TCP echo client example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/tcp/client.h"

#include <iostream>

class EchoClient : public CppServer::TCPClient
{
public:
    using CppServer::TCPClient::TCPClient;

protected:
    void onStarting() override
    {
        std::cout << "EchoClient starting..." << std::endl;
    }
    void onStarted() override
    {
        std::cout << "EchoClient started!" << std::endl;
    }
    void onStopping() override
    {
        std::cout << "EchoClient stopping..." << std::endl;
    }
    void onStopped() override
    {
        std::cout << "EchoClient stopped!" << std::endl;
    }

    void onConnected() override
    {
        std::cout << "EchoClient connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "EchoClient disconnected a session with Id " << id() << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "EchoClient caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new TCP echo client
    EchoClient client("127.0.0.1", 1234);

	// Start the client
	client.Start();

    // Connect the client
    client.Connect();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Send entered text to the echo server
        std::cout << "Outgoing: " << line << std::endl;
        client.Send(line.data(), line.size());
    }

    // Disconnect the client
    client.Disconnect();

	// Stop the client
	client.Stop();

    return 0;
}
