/*!
    \file ssl_chat_client.cpp
    \brief SSL chat client example
    \author Ivan Shynkarenka
    \date 01.01.2017
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/ssl_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::Asio::SSLClient
{
public:
    ChatClient(std::shared_ptr<CppServer::Asio::Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port)
        : CppServer::Asio::SSLClient(service, context, address, port)
    {
        _stop = false;
    }

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
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

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop;
};

int main(int argc, char** argv)
{
    // SSL server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // SSL server port
    int port = 2222;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "SSL server address: " << address << std::endl;
    std::cout << "SSL server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
    context->set_verify_mode(asio::ssl::verify_peer);
    context->load_verify_file("../tools/certificates/ca.pem");

    // Create a new SSL chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->DisconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
