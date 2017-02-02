//
// Created by Ivan Shynkarenka on 03.02.2017.
//

#include "catch.hpp"

#include "server/nanomsg/publisher_server.h"
#include "server/nanomsg/subscriber_client.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

class TestSubscriberClient : public SubscriberClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestSubscriberClient(const std::string& address, const std::string& topic = "")
        : SubscriberClient(address, topic),
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

class TestPublisherServer : public PublisherServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestPublisherServer(const std::string& address)
        : PublisherServer(address),
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

TEST_CASE("Nanomsg subscriber client & publisher server", "[CppServer][Nanomsg]")
{
    const std::string server_address = "tcp://*:6672";
    const std::string client_address = "tcp://localhost:6672";

    // Create and start Nanomsg publisher server
    auto server = std::make_shared<TestPublisherServer>(server_address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg subscriber client
    auto client = std::make_shared<TestSubscriberClient>(client_address, "foo");
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    while (client->socket().bytes_received() == 0)
    {
        // Publish messages to all subscribed clients
        server->Send("test");
        server->Send("footest");
        Thread::Yield();
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
    REQUIRE(server->socket().accepted_connections() == 1);
    REQUIRE(server->socket().messages_sent() > 0);
    REQUIRE(server->socket().bytes_sent() > 0);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() == 1);
    REQUIRE(client->socket().messages_received() > 0);
    REQUIRE(client->socket().bytes_received() > 0);
    REQUIRE(!client->error);
}

TEST_CASE("Nanomsg publish/subscribe random test", "[CppServer][Nanomsg]")
{
    const std::string server_address = "tcp://*:6673";
    const std::string client_address = "tcp://localhost:6673";

    // Create and start Nanomsg publisher server
    auto server = std::make_shared<TestPublisherServer>(server_address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<TestSubscriberClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Create a new client and connect
        if ((rand() % 100) == 0)
        {
            // Create and connect Nanomsg subscriber client
            auto client = std::make_shared<TestSubscriberClient>(client_address);
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
