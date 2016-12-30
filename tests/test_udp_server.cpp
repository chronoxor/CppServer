//
// Created by Ivan Shynkarenka on 27.12.2016.
//

#include "catch.hpp"

#include "server/asio/udp_client.h"
#include "server/asio/udp_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

class EchoUDPService : public Service
{
public:
    std::atomic<bool> thread_initialize;
    std::atomic<bool> thread_cleanup;
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> idle;
    std::atomic<bool> error;

    explicit EchoUDPService()
        : thread_initialize(false),
          thread_cleanup(false),
          started(false),
          stopped(false),
          idle(false),
          error(false)
    {
    }

protected:
    void onThreadInitialize() override { thread_initialize = true; }
    void onThreadCleanup() override { thread_cleanup = true; }
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onIdle() override { idle = true; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoUDPClient : public UDPClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoUDPClient(std::shared_ptr<EchoUDPService>& service, const std::string& address, int port)
        : UDPClient(service, address, port),
          connected(false),
          disconnected(false),
          received(0),
          sent(0),
          error(false)
    {
    }
    explicit EchoUDPClient(std::shared_ptr<EchoUDPService>& service, const std::string& address, int port, bool reuse_address)
        : UDPClient(service, address, port, reuse_address),
          connected(false),
          disconnected(false),
          received(0),
          sent(0),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override { received += size; }
    void onSent(const asio::ip::udp::endpoint& endpoint, size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoUDPServer : public UDPServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoUDPServer(std::shared_ptr<EchoUDPService> service, InternetProtocol protocol, int port)
        : UDPServer(service, protocol, port),
          started(false),
          stopped(false),
          received(0),
          sent(0),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override { Send(endpoint, buffer, size); received += size; }
    void onSent(const asio::ip::udp::endpoint& endpoint, size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

TEST_CASE("UDP server & client", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 2222;

    // Create and start Asio service
    auto service = std::make_shared<EchoUDPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoUDPServer>(service, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client = std::make_shared<EchoUDPClient>(service, address, port);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Send some data to the Echo server
    client->Send("test", 4);

    // Wait for all data processed...
    while (client->received != 4)
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client->Disconnect());
    while (client->IsConnected())
        Thread::Yield();

    // Stop the Echo server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the Asio service state
    REQUIRE(service->thread_initialize);
    REQUIRE(service->thread_cleanup);
    REQUIRE(service->started);
    REQUIRE(service->stopped);
    REQUIRE(!service->idle);
    REQUIRE(!service->error);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->received == 4);
    REQUIRE(server->sent == 4);
    REQUIRE(!server->error);

    // Check the Echo client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->received == 4);
    REQUIRE(client->sent == 4);
    REQUIRE(!client->error);
}

TEST_CASE("UDP server multicast", "[CppServer][Asio]")
{
    const std::string listen_address = "0.0.0.0";
    const std::string multicast_address = "239.255.0.1";
    const int multicast_port = 2223;

    // Create and start Asio service
    auto service = std::make_shared<EchoUDPService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoUDPServer>(service, InternetProtocol::IPv4, 0);
    REQUIRE(server->Start(multicast_address, multicast_port));
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoUDPClient>(service, listen_address, multicast_port, true);
    REQUIRE(client1->Connect());
    while (!client1->IsConnected())
        Thread::Yield();
    client1->JoinMulticastGroup(multicast_address);

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while (client1->received != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoUDPClient>(service, listen_address, multicast_port, true);
    REQUIRE(client2->Connect());
    while (!client2->IsConnected())
        Thread::Yield();
    client2->JoinMulticastGroup(multicast_address);

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 8) || (client2->received != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoUDPClient>(service, listen_address, multicast_port, true);
    REQUIRE(client3->Connect());
    while (!client3->IsConnected())
        Thread::Yield();
    client3->JoinMulticastGroup(multicast_address);

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 8) || (client3->received != 4))
        Thread::Yield();

    // Disconnect the Echo client
    client1->LeaveMulticastGroup(multicast_address);
    REQUIRE(client1->Disconnect());
    while (client1->IsConnected())
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 8))
        Thread::Yield();

    // Disconnect the Echo client
    client2->LeaveMulticastGroup(multicast_address);
    REQUIRE(client2->Disconnect());
    while (client2->IsConnected())
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 12))
        Thread::Yield();

    // Disconnect the Echo client
    client3->LeaveMulticastGroup(multicast_address);
    REQUIRE(client3->Disconnect());
    while (client3->IsConnected())
        Thread::Yield();

    // Stop the Echo server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the Asio service state
    REQUIRE(service->thread_initialize);
    REQUIRE(service->thread_cleanup);
    REQUIRE(service->started);
    REQUIRE(service->stopped);
    REQUIRE(service->idle);
    REQUIRE(!service->error);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->received == 0);
    REQUIRE(server->sent == 20);
    REQUIRE(!server->error);

    // Check the Echo client state
    REQUIRE(client1->received == 12);
    REQUIRE(client2->received == 12);
    REQUIRE(client3->received == 12);
    REQUIRE(client1->sent == 0);
    REQUIRE(client2->sent == 0);
    REQUIRE(client3->sent == 0);
    REQUIRE(!client1->error);
    REQUIRE(!client2->error);
    REQUIRE(!client3->error);
}


TEST_CASE("UDP server random test", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 1239;

    // Create and start Asio service
    auto service = std::make_shared<EchoUDPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoUDPServer>(service, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<EchoUDPClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Connect a new client
        if ((rand() % 100) == 0)
        {
            // Create and connect Echo client
            auto client = std::make_shared<EchoUDPClient>(service, address, port);
            client->Connect();
            clients.emplace_back(client);
        }
        // Disconnect the random client
        else if ((rand() % 100) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                client->Disconnect();
                clients.erase(clients.begin() + index);
            }
        }
        // Send a message from the random client
        else if ((rand() % 1) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                client->Send("test", 4);
            }
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Stop the Echo server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->received > 0);
    REQUIRE(server->sent > 0);
    REQUIRE(server->received == server->sent);
    REQUIRE(!server->error);
}

TEST_CASE("UDP multicast server random test", "[CppServer][Asio]")
{
    const std::string listen_address = "0.0.0.0";
    const std::string multicast_address = "239.255.0.1";
    const int multicast_port = 1240;

    // Create and start Asio service
    auto service = std::make_shared<EchoUDPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoUDPServer>(service, InternetProtocol::IPv4, 0);
    REQUIRE(server->Start(multicast_address, multicast_port));
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<EchoUDPClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Connect a new client
        if ((rand() % 100) == 0)
        {
            // Create and connect Echo client
            auto client = std::make_shared<EchoUDPClient>(service, listen_address, multicast_port, true);
            client->Connect();
            while (!client->IsConnected())
                Thread::Yield();
            client->JoinMulticastGroup(multicast_address);
            clients.emplace_back(client);
        }
        // Disconnect the random client
        else if ((rand() % 100) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                client->LeaveMulticastGroup(multicast_address);
                client->Disconnect();
                clients.erase(clients.begin() + index);
            }
        }
        // Multicast a message to all clients
        else if ((rand() % 10) == 0)
        {
            server->Multicast("test", 4);
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Stop the Echo server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->received == 0);
    REQUIRE(server->sent > 0);
    REQUIRE(!server->error);
}
