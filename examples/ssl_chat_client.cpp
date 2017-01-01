/*!
    \file ssl_chat_client.cpp
    \brief SSL chat client example
    \author Ivan Shynkarenka
    \date 01.01.2017
    \copyright MIT License
*/

#include "server/asio/ssl_client.h"

#include <iostream>

class ChatClient : public CppServer::Asio::SSLClient
{
public:
    explicit ChatClient(std::shared_ptr<CppServer::Asio::Service> service, asio::ssl::context& context, const std::string& address, int port)
        : SSLClient(service, context, address, port)
    {
        stream().set_verify_mode(asio::ssl::verify_none);
    }

protected:
    void onConnected() override
    {
        std::cout << "Chat SSL client connected a new session with Id " << id() << std::endl;
    }
    void onHandshaked() override
    {
        std::cout << "Chat SSL client handshaked a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Chat SSL client disconnected a session with Id " << id() << std::endl;

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
        std::cout << "Chat SSL client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // SSL server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // SSL server port
    int port = 1112;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "SSL server address: " << address << std::endl;
    std::cout << "SSL server port: " << port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create and prepare a new SSL client context
    asio::ssl::context context(asio::ssl::context::sslv23);

    // Create a new SSL chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

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

    // Stop the service
    service->Stop();

    return 0;
}
