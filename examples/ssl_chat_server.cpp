/*!
    \file ssl_chat_server.cpp
    \brief SSL chat server example
    \author Ivan Shynkarenka
    \date 30.12.2016
    \copyright MIT License
*/

#include "server/asio/ssl_server.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::SSLServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::SSLServer<ChatServer, ChatSession>::SSLServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::SSLSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::SSLSession<ChatServer, ChatSession>::SSLSession;

protected:
    void onHandshaked() override
    {
        std::cout << "Chat SSL session with Id " << id() << " handshaked!" << std::endl;

        // Send invite message
        std::string message("Hello from SSL chat! Please send a message or '!' for disconnect!");
        Send(message.data(), message.size());
    }
    void onDisconnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::string messsage((const char*)buffer, size);
        std::cout << "Incoming: " << messsage << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(buffer, size);

        // If the buffer starts with '!' the disconnect the current session
        if (messsage == "!")
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // SSL server port
    int port = 1112;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "SSL server port: " << port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create and prepare a new SSL server context
    asio::ssl::context context(asio::ssl::context::sslv23);
    context.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    context.set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "123qwe!"; });
    context.use_certificate_chain_file("../tools/certificates/server.pem");
    context.use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context.use_tmp_dh_file("../tools/certificates/dh2048.pem");

    // Create a new SSL chat server
    auto server = std::make_shared<ChatServer>(service, context, CppServer::Asio::InternetProtocol::IPv4, port);

    // Start the server
    server->Start();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line.data(), line.size());
    }

    // Stop the server
    server->Stop();

    // Stop the service
    service->Stop();

    return 0;
}
