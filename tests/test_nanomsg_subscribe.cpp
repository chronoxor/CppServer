//
// Created by Ivan Shynkarenka on 03.02.2017
//

#include "catch.hpp"

#include "server/nanomsg/subscribe_server.h"
#include "server/nanomsg/subscribe_client.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

class TestSubscribeClient : public SubscribeClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestSubscribeClient(const std::string& address, const std::string& topic = "")
        : SubscribeClient(address, topic),
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

class TestSubscribeServer : public SubscribeServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestSubscribeServer(const std::string& address)
        : SubscribeServer(address),
          started(false),
          stopped(false),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onError(int error, const std::string& message) override { error = true; }
};

TEST_CASE("Nanomsg subscribe server & client", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6672";

    // Create and start Nanomsg subscribe server
    auto server = std::make_shared<TestSubscribeServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg subscribe client
    auto client = std::make_shared<TestSubscribeClient>(address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Sleep for a while...
    Thread::Sleep(1000);

    // Publish messages to all subscribed clients
    server->Send("test");

    // Wait for all data processed...
    while (client->socket().bytes_received() != 4)
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
    REQUIRE(server->socket().messages_sent() == 1);
    REQUIRE(server->socket().bytes_sent() == 4);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() == 1);
    REQUIRE(client->socket().messages_received() == 1);
    REQUIRE(client->socket().bytes_received() == 4);
    REQUIRE(!client->error);
}

TEST_CASE("Nanomsg subscribe random test", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6673";

    // Create and start Nanomsg subscribe server
    auto server = std::make_shared<TestSubscribeServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<TestSubscribeClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Create a new client and connect
        if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Nanomsg subscribe client
                auto client = std::make_shared<TestSubscribeClient>(address);
                client->Connect();
                clients.emplace_back(client);
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
                    client->Disconnect();
                else
                    client->Connect();
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
                    client->Reconnect();
            }
        }
        // Publish a message to all subscribed clients
        else if ((rand() % 1) == 0)
        {
            server->Send("test");
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Stop the server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Check the server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->socket().accepted_connections() > 0);
    REQUIRE(server->socket().messages_sent() > 0);
    REQUIRE(server->socket().bytes_sent() > 0);
    REQUIRE(!server->error);
}
