/*!
    \file proto_client.cpp
    \brief Simple protocol client example
    \author Ivan Shynkarenka
    \date 05.01.2022
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/tcp_client.h"
#include "string/format.h"
#include "threads/thread.h"

#include "../proto/simple_protocol.h"

#include <atomic>
#include <iostream>

class SimpleProtoClient : public CppServer::Asio::TCPClient, public FBE::simple::Client
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

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
        std::cout << "Simple protocol client connected a new session with Id " << id() << std::endl;

        // Reset FBE protocol buffers
        reset();
    }

    void onDisconnected() override
    {
        std::cout << "Simple protocol client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol handlers
    void onReceive(const ::simple::DisconnectRequest& request) override { Client::onReceive(request); std::cout << "Received: " << request << std::endl; DisconnectAsync(); }
    void onReceive(const ::simple::SimpleResponse& response) override { Client::onReceive(response); std::cout << "Received: " << response << std::endl; }
    void onReceive(const ::simple::SimpleReject& reject) override { Client::onReceive(reject); std::cout << "Received: " << reject << std::endl; }
    void onReceive(const ::simple::SimpleNotify& notify) override { Client::onReceive(notify); std::cout << "Received: " << notify << std::endl; }

    // Protocol implementation
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }

private:
    std::atomic<bool> _stop{false};
};

int main(int argc, char** argv)
{
    // Simple protocol server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // Simple protocol server port
    int port = 4444;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "Simple protocol server address: " << address << std::endl;
    std::cout << "Simple protocol server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new simple protocol client
    auto client = std::make_shared<SimpleProtoClient>(service, address, port);

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

        // Reconnect the client
        if (line == "!")
        {
            std::cout << "Client reconnecting...";
            client->IsConnected() ? client->ReconnectAsync() : client->ConnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send request to the simple protocol server
        simple::SimpleRequest request;
        request.Message = line;
        auto response = client->request(request).get();

        // Show string hash calculation result
        std::cout << "Hash of '" << line << "' = " << CppCommon::format("0x{:08X}", response.Hash) << std::endl;
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
