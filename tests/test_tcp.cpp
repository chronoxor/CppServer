//
// Created by Ivan Shynkarenka on 16.12.2016
//

#include "test.h"

#include "server/asio/tcp_client.h"
#include "server/asio/tcp_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

namespace {

class EchoTCPService : public Service
{
public:
    using Service::Service;

protected:
    void onThreadInitialize() override { thread_initialize = true; }
    void onThreadCleanup() override { thread_cleanup = true; }
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onIdle() override { idle = true; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> thread_initialize{false};
    std::atomic<bool> thread_cleanup{false};
    std::atomic<bool> started{false};
    std::atomic<bool> stopped{false};
    std::atomic<bool> idle{false};
    std::atomic<bool> errors{false};
};

class EchoTCPClient : public TCPClient
{
public:
    using TCPClient::TCPClient;

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errors{false};
};

class EchoTCPSession : public TCPSession
{
public:
    using TCPSession::TCPSession;

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(const void* buffer, size_t size) override { SendAsync(buffer, size); }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errors{false};
};

class EchoTCPServer : public TCPServer
{
public:
    using TCPServer::TCPServer;

protected:
    std::shared_ptr<TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) override { return std::make_shared<EchoTCPSession>(server); }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<TCPSession>& session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<TCPSession>& session) override { disconnected = true; --clients; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> started{false};
    std::atomic<bool> stopped{false};
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<size_t> clients{0};
    std::atomic<bool> errors{false};
};

} // namespace

TEST_CASE("TCP server test", "[CppServer][TCP]")
{
    const std::string address = "127.0.0.1";
    const int port = 1111;

    // Create and start Asio service
    auto service = std::make_shared<EchoTCPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoTCPServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client = std::make_shared<EchoTCPClient>(service, address, port);
    REQUIRE(client->ConnectAsync());
    while (!client->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Send a message to the Echo server
    client->SendAsync("test");

    // Wait for all data processed...
    while (client->bytes_received() != 4)
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client->DisconnectAsync());
    while (client->IsConnected() || (server->clients != 0))
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
    REQUIRE(!service->errors);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() == 4);
    REQUIRE(server->bytes_received() == 4);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->bytes_sent() == 4);
    REQUIRE(client->bytes_received() == 4);
    REQUIRE(!client->errors);
}

TEST_CASE("TCP server multicast test", "[CppServer][TCP]")
{
    const std::string address = "127.0.0.1";
    const int port = 1112;

    // Create and start Asio service
    auto service = std::make_shared<EchoTCPService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoTCPServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoTCPClient>(service, address, port);
    REQUIRE(client1->ConnectAsync());
    while (!client1->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while (client1->bytes_received() != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoTCPClient>(service, address, port);
    REQUIRE(client2->ConnectAsync());
    while (!client2->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 8) || (client2->bytes_received() != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoTCPClient>(service, address, port);
    REQUIRE(client3->ConnectAsync());
    while (!client3->IsConnected() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 8) || (client3->bytes_received() != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->DisconnectAsync());
    while (client1->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->DisconnectAsync());
    while (client2->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 12))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client3->DisconnectAsync());
    while (client3->IsConnected() || (server->clients != 0))
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
    REQUIRE(!service->errors);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() == 36);
    REQUIRE(server->bytes_received() == 0);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client1->bytes_sent() == 0);
    REQUIRE(client2->bytes_sent() == 0);
    REQUIRE(client3->bytes_sent() == 0);
    REQUIRE(client1->bytes_received() == 12);
    REQUIRE(client2->bytes_received() == 12);
    REQUIRE(client3->bytes_received() == 12);
    REQUIRE(!client1->errors);
    REQUIRE(!client2->errors);
    REQUIRE(!client3->errors);
}

TEST_CASE("TCP server random test", "[CppServer][TCP]")
{
    const std::string address = "127.0.0.1";
    const int port = 1113;

    // Create and start Asio service
    auto service = std::make_shared<EchoTCPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoTCPServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<EchoTCPClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Disconnect all clients
        if ((rand() % 1000) == 0)
        {
            server->DisconnectAll();
        }
        // Create a new client and connect
        else if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Echo client
                auto client = std::make_shared<EchoTCPClient>(service, address, port);
                clients.emplace_back(client);
                client->ConnectAsync();
                while (!client->IsConnected())
                    Thread::Yield();
            }
        }
        // Connect/Disconnect the random client
        else if ((rand() % 100) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsConnected())
                {
                    client->DisconnectAsync();
                    while (client->IsConnected())
                        Thread::Yield();
                }
                else
                {
                    client->ConnectAsync();
                    while (!client->IsConnected())
                        Thread::Yield();
                }
            }
        }
        // Reconnect the random client
        else if ((rand() % 100) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsConnected())
                {
                    client->ReconnectAsync();
                    while (!client->IsConnected())
                        Thread::Yield();
                }
            }
        }
        // Multicast a message to all clients
        else if ((rand() % 10) == 0)
        {
            server->Multicast("test");
        }
        // Send a message from the random client
        else if ((rand() % 1) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsConnected())
                    client->SendAsync("test");
            }
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Disconnect clients
    for (auto& client : clients)
    {
        client->DisconnectAsync();
        while (client->IsConnected())
            Thread::Yield();
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
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->errors);
}
