/*!
    \file websocket_ssl_chat_server.cpp
    \brief WebSocket SSL chat server example
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/websocket_ssl_server.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::WebSocketSSLServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSSLServer<ChatServer, ChatSession>::WebSocketSSLServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket SSL server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::WebSocketSSLSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSSLSession<ChatServer, ChatSession>::WebSocketSSLSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocket SSL session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket SSL chat! Please send a message or '!' to disconnect the client!");
        Send(message);
    }

    void onDisconnected() override
    {
        std::cout << "Chat WebSocket SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(CppServer::Asio::WebSocketSSLMessage message) override
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
        std::cout << "Chat WebSocket SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket SSL server port
    int port = 5555;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "WebSocket server port: " << port << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    service->Start();

    // Create and prepare a new SSL server context
    std::shared_ptr<asio::ssl::context> context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    context->set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    context->set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new WebSocket SSL chat server
    auto server = std::make_shared<ChatServer>(service, context, CppServer::Asio::InternetProtocol::IPv4, port);

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
