/*!
    \file tcp_chat_client.cpp
    \brief TCP chat client example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/asio/tcp_client.h"

#include <iostream>

class ChatClient : public CppServer::Asio::TCPClient
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

protected:
    void onConnected() override
    {
        std::cout << "Chat TCP client connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Chat TCP client disconnected a session with Id " << id() << std::endl;

        // Try to wait for a while
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // TCP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // TCP server port
    int port = 1234;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "TCP server address: " << address << std::endl;
    std::cout << "TCP server port: " << port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the chat service
    service->Start();

    // Create a new TCP chat client
    auto client = std::make_shared<ChatClient>(service, address, port);

    // Connect the client
    client->Connect();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Send the entered text to the chat server
        client->Send(line.data(), line.size());
    }

    // Disconnect the client
    client->Disconnect();

    // Stop the chat service
    service->Stop();

    return 0;
}
