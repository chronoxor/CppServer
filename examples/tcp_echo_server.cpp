/*!
    \file tcp_echo_server.cpp
    \brief TCP echo server example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/tcp/server.h"
#include "server/tcp/session.h"

#include <iostream>

class EchoServer;

class EchoSession : public CppServer::TCPSession<EchoServer, EchoSession>
{
public:
    using CppServer::TCPSession<EchoServer, EchoSession>::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "EchoSession with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Please send something to echo server or send '0' for disconnect!");
        Send(message.data(), message.size());
    }
    void onDisconnected() override
    {
        std::cout << "EchoSession with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        // Send the buffer back to client
        Send(buffer, size);

        // If the buffer starts with '0' the disconnect the client
        if (*(const uint8_t*)buffer == '0')
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "EchoSession caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoServer : public CppServer::TCPServer<EchoServer, EchoSession>
{
public:
    using CppServer::TCPServer<EchoServer, EchoSession>::TCPServer;

protected:
    void onStarting() override
    {
        std::cout << "EchoServer starting..." << std::endl;
    }
    void onStarted() override
    {
        std::cout << "EchoServer started!" << std::endl;
    }
    void onStopping() override
    {
        std::cout << "EchoServer stopping..." << std::endl;
    }
    void onStopped() override
    {
        std::cout << "EchoServer stopped!" << std::endl;
    }

    void onConnected(std::shared_ptr<EchoSession> session) override
    {
        std::cout << "EchoServer connected a new session with Id " << session->id() << std::endl;
    }
    void onDisconnected(std::shared_ptr<EchoSession> session) override
    {
        std::cout << "EchoServer disconnected a session with Id " << session->id() << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "EchoServer caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new TCP echo server
    EchoServer server(CppServer::InternetProtocol::IPv4, 1234);

    // Start the server
    server.Start();

    // Wait for input
    std::cin.get();

    // Stop the server
    server.Stop();

    return 0;
}
