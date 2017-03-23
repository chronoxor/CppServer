//
// Created by Ivan Shynkarenka on 11.01.2017
//

#include "catch.hpp"

#include "errors/exceptions_handler.h"
#include "server/asio/websocket_client.h"
#include "server/asio/websocket_server.h"
#include "system/stack_trace_manager.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

class EchoWebSocketService : public Service
{
public:
    std::atomic<bool> thread_initialize;
    std::atomic<bool> thread_cleanup;
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> idle;
    std::atomic<bool> error;

    explicit EchoWebSocketService()
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

class EchoWebSocketClient : public WebSocketClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit EchoWebSocketClient(std::shared_ptr<EchoWebSocketService> service, const std::string& uri)
        : WebSocketClient(service, uri),
          connected(false),
          disconnected(false),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoWebSocketServer;

class EchoWebSocketSession : public WebSocketSession<EchoWebSocketServer, EchoWebSocketSession>
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit EchoWebSocketSession(std::shared_ptr<WebSocketServer<EchoWebSocketServer, EchoWebSocketSession>> server)
        : WebSocketSession<EchoWebSocketServer, EchoWebSocketSession>(server),
          connected(false),
          disconnected(false),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(const WebSocketMessage& message) override { Send(message); }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoWebSocketServer : public WebSocketServer<EchoWebSocketServer, EchoWebSocketSession>
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> clients;
    std::atomic<bool> error;

    explicit EchoWebSocketServer(std::shared_ptr<EchoWebSocketService> service, InternetProtocol protocol, int port)
        : WebSocketServer<EchoWebSocketServer, EchoWebSocketSession>(service, protocol, port),
          started(false),
          stopped(false),
          connected(false),
          disconnected(false),
          clients(0),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<EchoWebSocketSession>& session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<EchoWebSocketSession>& session) override { disconnected = true; --clients; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

TEST_CASE("WebSocket server", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 4444;
    const std::string uri = "ws://" + address + ":" + std::to_string(port);

    // Create and start Asio service
    auto service = std::make_shared<EchoWebSocketService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoWebSocketServer>(service, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client = std::make_shared<EchoWebSocketClient>(service, uri);
    REQUIRE(client->Connect());
    while (!client->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Send a message to the Echo server
    client->Send("test");

    // Wait for all data processed...
    while (client->bytes_received() != 4)
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
    REQUIRE(server->bytes_sent() == 4);
    REQUIRE(server->bytes_received() == 4);
    REQUIRE(!server->error);

    // Check the Echo client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->bytes_sent() == 4);
    REQUIRE(client->bytes_received() == 4);
    REQUIRE(!client->error);
}

TEST_CASE("WebSocket server multicast", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 4445;
    const std::string uri = "ws://" + address + ":" + std::to_string(port);

    // Create and start Asio service
    auto service = std::make_shared<EchoWebSocketService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoWebSocketServer>(service, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoWebSocketClient>(service, uri);
    REQUIRE(client1->Connect());
    while (!client1->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while (client1->bytes_received() != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoWebSocketClient>(service, uri);
    REQUIRE(client2->Connect());
    while (!client2->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 8) || (client2->bytes_received() != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoWebSocketClient>(service, uri);
    REQUIRE(client3->Connect());
    while (!client3->IsConnected() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 8) || (client3->bytes_received() != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->Disconnect());
    while (client1->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->Disconnect());
    while (client2->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 12))
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
    REQUIRE(server->bytes_sent() == 36);
    REQUIRE(server->bytes_received() == 0);
    REQUIRE(!server->error);

    // Check the Echo client state
    REQUIRE(client1->bytes_sent() == 0);
    REQUIRE(client2->bytes_sent() == 0);
    REQUIRE(client3->bytes_sent() == 0);
    REQUIRE(client1->bytes_received() == 12);
    REQUIRE(client2->bytes_received() == 12);
    REQUIRE(client3->bytes_received() == 12);
    REQUIRE(!client1->error);
    REQUIRE(!client2->error);
    REQUIRE(!client3->error);
}

TEST_CASE("WebSocket server random test", "[CppServer][Asio]")
{
    // Initialize stack trace manager of the current process
    CppCommon::StackTraceManager::Initialize();
    // Setup exceptions handler for the current process
    CppCommon::ExceptionsHandler::SetupProcess();

    const std::string address = "127.0.0.1";
    const int port = 4446;
    const std::string uri = "ws://" + address + ":" + std::to_string(port);

    // Create and start Asio service
    auto service = std::make_shared<EchoWebSocketService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoWebSocketServer>(service, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<EchoWebSocketClient>> clients;

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
                auto client = std::make_shared<EchoWebSocketClient>(service, uri);
                clients.emplace_back(client);
                client->Connect();
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
                    client->Disconnect();
                    while (client->IsConnected())
                        Thread::Yield();
                }
                else
                {
                    client->Connect();
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
                    client->Reconnect();
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
                    client->Send("test");
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
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->error);
}
