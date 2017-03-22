//
// Created by Ivan Shynkarenka on 02.02.2017
//

#include "catch.hpp"

#include "server/nanomsg/pair_client.h"
#include "server/nanomsg/pair_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

class TestPairClient : public PairClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestPairClient(const std::string& address)
        : PairClient(address),
          connected(false),
          disconnected(false),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onError(int error, const std::string& message) override { error = true; }
};

class TestPairServer : public PairServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestPairServer(const std::string& address)
        : PairServer(address),
          started(false),
          stopped(false),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onReceived(Message& message) override { Send(message); }
    void onError(int error, const std::string& message) override { error = true; }
};

TEST_CASE("Nanomsg pair", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6668";

    // Create and start Nanomsg pair server
    auto server = std::make_shared<TestPairServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg pair client
    auto client = std::make_shared<TestPairClient>(address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Send a message to the server
    client->Send("test");

    // Send a message from the server
    server->Send("test");

    // Wait for all data processed...
    while ((server->socket().bytes_received() != 4) || (client->socket().bytes_received() != 8))
        Thread::Yield();

    // Disconnect the client
    REQUIRE(client->Disconnect());
    while (client->IsConnected())
        Thread::Yield();

    // Stop the server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Check the server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->socket().accepted_connections() == 1);
    REQUIRE(server->socket().messages_sent() == 2);
    REQUIRE(server->socket().messages_received() == 1);
    REQUIRE(server->socket().bytes_sent() == 8);
    REQUIRE(server->socket().bytes_received() == 4);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() == 1);
    REQUIRE(client->socket().messages_sent() == 1);
    REQUIRE(client->socket().messages_received() == 2);
    REQUIRE(client->socket().bytes_sent() == 4);
    REQUIRE(client->socket().bytes_received() == 8);
    REQUIRE(!client->error);
}

TEST_CASE("Nanomsg pair random test", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6669";

    // Create and start Nanomsg pair server
    auto server = std::make_shared<TestPairServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg pair client
    auto client = std::make_shared<TestPairClient>(address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Reconnect the client
        if ((rand() % 100) == 0)
        {
            client->Reconnect();
            while (!client->IsConnected())
                Thread::Yield();
        }
        // Send a message from the client & server
        else if ((rand() % 1) == 0)
        {
            if (client->IsConnected())
                client->Send("test");

            if (server->IsStarted())
                server->Send("test");
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Disconnect the client
    REQUIRE(client->Disconnect());
    while (client->IsConnected())
        Thread::Yield();

    // Stop the server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Check the server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->socket().accepted_connections() > 0);
    REQUIRE(server->socket().messages_sent() > 0);
    REQUIRE(server->socket().messages_received() > 0);
    REQUIRE(server->socket().bytes_sent() > 0);
    REQUIRE(server->socket().bytes_received() > 0);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() > 0);
    REQUIRE(client->socket().messages_sent() > 0);
    REQUIRE(client->socket().messages_received() > 0);
    REQUIRE(client->socket().bytes_sent() > 0);
    REQUIRE(client->socket().bytes_received() > 0);
    REQUIRE(!client->error);
}
