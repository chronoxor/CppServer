/*!
    \file tcp_chat_server.cpp
    \brief TCP chat server example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/tcp/server.h"
#include "server/tcp/session.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::TCPServer<ChatServer, ChatSession>
{
public:
    using CppServer::TCPServer<ChatServer, ChatSession>::TCPServer;

protected:
    void onStarting() override
    {
        std::cout << "Chat server starting..." << std::endl;
    }
    void onStarted() override
    {
        std::cout << "Chat server started!" << std::endl;
    }
    void onStopping() override
    {
        std::cout << "Chat server stopping..." << std::endl;
    }
    void onStopped() override
    {
        std::cout << "Chat server stopped!" << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::TCPSession<ChatServer, ChatSession>
{
public:
    using CppServer::TCPSession<ChatServer, ChatSession>::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello in chat! Please send a message or '!' for disconnect!");
        Send(message.data(), message.size());
    }
    void onDisconnected() override
    {
        std::cout << "Chat session with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::string messsage((const char*)buffer, size);
        std::cout << "Incoming: " << messsage << std::endl;

        // Broadcast message to all sessions
        server().Broadcast(buffer, size);

        // If the buffer starts with '!' the disconnect the session
        if (messsage == "!")
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Server port
    int port = 1234;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "Server port: " << port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new TCP chat server
    ChatServer server(CppServer::InternetProtocol::IPv4, port);

    // Start the server
    server.Start();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Send the entered text to the chat server
        line = "(admin) " + line;
        server.Broadcast(line.data(), line.size());
    }

    // Stop the server
    server.Stop();

    return 0;
}
