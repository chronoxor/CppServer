/*!
    \file ssl_chat_server.cpp
    \brief SSL chat server example
    \author Ivan Shynkarenka
    \date 30.12.2016
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/ssl_server.h"

#include <iostream>

class ChatSession : public CppServer::Asio::SSLSession
{
public:
    using CppServer::Asio::SSLSession::SSLSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " connected!" << std::endl;
    }

    void onHandshaked() override
    {
        std::cout << "Chat SSL session with Id " << id() << " handshaked!" << std::endl;

        // Send invite message
        std::string message("Hello from SSL chat! Please send a message or '!' to disconnect the client!");
        SendAsync(message.data(), message.size());
    }

    void onDisconnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Disconnect();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatServer : public CppServer::Asio::SSLServer
{
public:
    using CppServer::Asio::SSLServer::SSLServer;

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer>& server) override
    {
        return std::make_shared<ChatSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // SSL server port
    int port = 2222;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "SSL server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new SSL chat server
    auto server = std::make_shared<ChatServer>(service, context, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

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
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
