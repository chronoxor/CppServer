/*!
    \file proto_server.cpp
    \brief Simple protocol server example
    \author Ivan Shynkarenka
    \date 05.01.2022
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/tcp_server.h"

#include "../proto/simple_protocol.h"

#include <iostream>

class SimpleProtoSession : public CppServer::Asio::TCPSession, public FBE::simple::Sender, public FBE::simple::Receiver
{
public:
    using CppServer::Asio::TCPSession::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "Simple protocol session with Id " << id() << " connected!" << std::endl;

        // Send invite notification
        simple::SimpleNotify notify;
        notify.Notification = "Hello from Simple protocol server! Please send a message or '!' to disconnect the client!";
        send(notify);
    }

    void onDisconnected() override
    {
        std::cout << "Simple protocol session with Id " << id() << " disconnected!" << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol handlers
    void onReceive(const ::simple::DisconnectRequest& request) override { Disconnect(); }
    void onReceive(const ::simple::SimpleRequest& request) override
    {
        std::cout << "Received: " << request << std::endl;

        // Validate request
        if (request.Message.empty())
        {
            // Send reject
            simple::SimpleReject reject;
            reject.id = request.id;
            reject.Error = "Request message is empty!";
            send(reject);
            return;
        }

        static std::hash<std::string> hasher;

        // Send response
        simple::SimpleResponse response;
        response.id = request.id;
        response.Hash = (uint32_t)hasher(request.Message);
        response.Length = (uint32_t)request.Message.size();
        send(response);
    }

    // Protocol implementation
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }
};

class SimpleProtoServer : public CppServer::Asio::TCPServer, public FBE::simple::Sender
{
public:
    using CppServer::Asio::TCPServer::TCPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override
    {
        return std::make_shared<SimpleProtoSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol implementation
    size_t onSend(const void* data, size_t size) override { Multicast(data, size); return size; }
};

int main(int argc, char** argv)
{
    // Simple protocol server port
    int port = 4444;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "Simple protocol server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new simple protocol server
    auto server = std::make_shared<SimpleProtoServer>(service, port);

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

        // Multicast admin notification to all sessions
        simple::SimpleNotify notify;
        notify.Notification = "(admin) " + line;
        server->send(notify);
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
