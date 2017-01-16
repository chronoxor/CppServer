/*!
    \file websocket_chat_server.cpp
    \brief WebSocket chat server example
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#include "server/asio/websocket_server.h"

#include "asio_service.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::WebSocketServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketServer<ChatServer, ChatSession>::WebSocketServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::WebSocketSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSession<ChatServer, ChatSession>::WebSocketSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocketCP session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket chat! Please send a message or '!' to disconnect the client!");
        Send(message);
    }
    void onDisconnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(CppServer::Asio::WebSocketMessage message) override
    {
        std::cout << "Incoming: " << message->get_raw_payload() << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message->get_payload() == "!")
            Disconnect();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket server port
    int port = 4444;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "WebSocket server port: " << port << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    service->Start();

    // Create a new WebSocket chat server
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
