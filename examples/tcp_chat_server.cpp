/*!
    \file tcp_chat_server.cpp
    \brief TCP chat server example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/asio/tcp_server.h"

#include "asio_service.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::TCPServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::TCPServer<ChatServer, ChatSession>::TCPServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::TCPSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::TCPSession<ChatServer, ChatSession>::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from TCP chat! Please send a message or '!' to disconnect the client!");
        Send(message);
    }
    void onDisconnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // TCP server port
    int port = 1111;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "TCP server port: " << port << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    service->Start();

    // Create a new TCP chat server
    auto server = std::make_shared<ChatServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

    // Start the server
    server->Start();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line);
    }

    // Stop the server
    server->Stop();

    // Stop the service
    service->Stop();

    return 0;
}
