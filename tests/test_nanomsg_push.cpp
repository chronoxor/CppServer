//
// Created by Ivan Shynkarenka on 01.02.2017.
//

#include "catch.hpp"

#include "server/nanomsg/push_server.h"
#include "server/nanomsg/push_client.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

class TestPushClient : public PushClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestPushClient(const std::string& address)
        : PushClient(address),
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

class TestPushServer : public PushServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestPushServer(const std::string& address)
        : PushServer(address),
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

TEST_CASE("Nanomsg push server & client", "[CppServer][Nanomsg]")
{
    const std::string server_address = "tcp://*:6666";
    const std::string client_address = "tcp://localhost:6666";

    // Create and start Nanomsg push server
    auto server = std::make_shared<TestPushServer>(server_address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg push client
    auto client = std::make_shared<TestPushClient>(client_address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Send a message to the server
    client->Send("test");

    // Wait for all data processed...
    while (server->socket().bytes_received() != 4)
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
    REQUIRE(server->socket().messages_received() == 1);
    REQUIRE(server->socket().bytes_received() == 4);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() == 1);
    REQUIRE(client->socket().messages_sent() == 1);
    REQUIRE(client->socket().bytes_sent() == 4);
    REQUIRE(!client->error);
}

TEST_CASE("Nanomsg push random test", "[CppServer][Nanomsg]")
{
    const std::string server_address = "tcp://*:6667";
    const std::string client_address = "tcp://localhost:6667";

    // Create and start Nanomsg push server
    auto server = std::make_shared<TestPushServer>(server_address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<TestPushClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Create a new client and connect
        if ((rand() % 100) == 0)
        {
            // Create and connect Nanomsg push client
            auto client = std::make_shared<TestPushClient>(client_address);
            client->Connect();
            clients.emplace_back(client);
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

    // Stop the server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Check the server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->socket().accepted_connections() > 0);
    REQUIRE(server->socket().messages_received() > 0);
    REQUIRE(server->socket().bytes_received() > 0);
    REQUIRE(!server->error);
}
