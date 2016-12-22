//
// Created by Ivan Shynkarenka on 16.12.2016.
//

#include "catch.hpp"

#include "server/asio/client.h"
#include "server/asio/tcp_server.h"
#include "server/asio/tcp_session.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

class EchoService : public Service
{
public:
    std::atomic<bool> thread_initialize;
    std::atomic<bool> thread_cleanup;
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> idle;
    std::atomic<bool> error;

    explicit EchoService()
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

class EchoClient : public TCPClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoClient(std::shared_ptr<EchoService>& service, const std::string& address, int port)
        : TCPClient(service, address, port),
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
    size_t onReceived(const void* buffer, size_t size) override { received += size; return size; }
    void onSent(size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoServer;

class EchoSession : public TCPSession<EchoServer, EchoSession>
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoSession(asio::ip::tcp::socket socket)
        : TCPSession<EchoServer, EchoSession>(std::move(socket)),
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
    size_t onReceived(const void* buffer, size_t size) override { Send(buffer, size); received += size; return size; }
    void onSent(size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoServer : public TCPServer<EchoServer, EchoSession>
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> clients;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoServer(std::shared_ptr<EchoService> service, InternetProtocol protocol, int port)
        : TCPServer<EchoServer, EchoSession>(service, protocol, port),
          started(false),
          stopped(false),
          connected(false),
          disconnected(false),
          clients(0),
          received(0),
          sent(0),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<EchoSession> session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<EchoSession> session) override { disconnected = true; --clients; received += session->received; sent += session->sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

TEST_CASE("TCP server & client", "[CppServer]")
{
    // Create and start Asio service
    auto service = std::make_shared<EchoService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoServer>(service, InternetProtocol::IPv4, 1234);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client = std::make_shared<EchoClient>(service, "127.0.0.1", 1234);
    REQUIRE(client->Connect());
    while (!client->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Send some data to the Echo server
    client->Send("test", 4);

    // Wait for all data processed...
    while (client->received != 4)
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client->Disconnect());
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
    REQUIRE(!service->error);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
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

TEST_CASE("TCP server multicast", "[CppServer]")
{
    // Create and start Asio service
    auto service = std::make_shared<EchoService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoServer>(service, InternetProtocol::IPv4, 1235);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoClient>(service, "127.0.0.1", 1235);
    REQUIRE(client1->Connect());
    while (!client1->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while (client1->received != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoClient>(service, "127.0.0.1", 1235);
    REQUIRE(client2->Connect());
    while (!client2->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 8) || (client2->received != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoClient>(service, "127.0.0.1", 1235);
    REQUIRE(client3->Connect());
    while (!client3->IsConnected() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 8) || (client3->received != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->Disconnect());
    while (client1->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->Disconnect());
    while (client2->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 12))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client3->Disconnect());
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
    REQUIRE(!service->error);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->received == 0);
    REQUIRE(server->sent == 36);
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


TEST_CASE("TCP server random test", "[CppServer]")
{
    // Create and start Asio service
    auto service = std::make_shared<EchoService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoServer>(service, InternetProtocol::IPv4, 1236);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<EchoClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Connect a new client
        if ((rand() % 100) == 0)
        {
            // Create and connect Echo client
            auto client = std::make_shared<EchoClient>(service, "127.0.0.1", 1236);
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
        // Multicast a message to all clients
        else if ((rand() % 10) == 0)
        {
            server->Multicast("test", 4);
        }
        // Disconnect all clients
        else if ((rand() % 1000) == 0)
        {
            server->DisconnectAll();
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
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->received > 0);
    REQUIRE(server->sent > 0);
    REQUIRE(server->received == server->sent);
    REQUIRE(!server->error);
}
