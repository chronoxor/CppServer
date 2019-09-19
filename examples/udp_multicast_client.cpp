/*!
    \file udp_multicast_client.cpp
    \brief UDP multicast client example
    \author Ivan Shynkarenka
    \date 27.12.2016
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/udp_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class MulticastClient : public CppServer::Asio::UDPClient
{
public:
    MulticastClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, const std::string& multicast, int port)
        : CppServer::Asio::UDPClient(service, address, port),
          _multicast(multicast)
    {
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
        std::cout << "Multicast UDP client connected a new session with Id " << id() << std::endl;

        // Join UDP multicast group
        JoinMulticastGroup(_multicast);

        // Start receive datagrams
        ReceiveAsync();
    }

    void onDisconnected() override
    {
        std::cout << "Multicast UDP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;

        // Continue receive datagrams
        ReceiveAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Multicast UDP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop{false};
    std::string _multicast;
};

int main(int argc, char** argv)
{
    // UDP listen address
    std::string listen_address = "0.0.0.0";
    if (argc > 1)
        listen_address = argv[1];

    // UDP multicast address
    std::string multicast_address = "239.255.0.1";
    if (argc > 2)
        multicast_address = argv[2];

    // UDP multicast port
    int multicast_port = 3334;
    if (argc > 3)
        multicast_port = std::atoi(argv[3]);

    std::cout << "UDP listen address: " << listen_address << std::endl;
    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast client
    auto client = std::make_shared<MulticastClient>(service, listen_address, multicast_address, multicast_port);
    client->SetupMulticast(true);

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
