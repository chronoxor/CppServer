//
// Created by Ivan Shynkarenka on 02.02.2017
//

#include "catch.hpp"

#include "server/nanomsg/request_server.h"
#include "server/nanomsg/request_client.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

namespace {

class TestRequestClient : public RequestClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestRequestClient(const std::string& address)
        : RequestClient(address),
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

class TestRequestServer : public RequestServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestRequestServer(const std::string& address)
        : RequestServer(address),
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

} // namespace

TEST_CASE("Nanomsg request", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6670";

    // Create and start Nanomsg request server
    auto server = std::make_shared<TestRequestServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg request client
    auto client = std::make_shared<TestRequestClient>(address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Request a message to the server
    Message message = client->Request("test");

    // Wait for all data processed...
    while ((server->socket().bytes_received() != 4) || (client->socket().bytes_received() != 4))
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
    REQUIRE(server->socket().messages_received() == 1);
    REQUIRE(server->socket().bytes_sent() == 4);
    REQUIRE(server->socket().bytes_received() == 4);
    REQUIRE(!server->error);

    // Check the client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->socket().established_connections() == 1);
    REQUIRE(client->socket().messages_sent() == 1);
    REQUIRE(client->socket().messages_received() == 1);
    REQUIRE(client->socket().bytes_sent() == 4);
    REQUIRE(client->socket().bytes_received() == 4);
    REQUIRE(!client->error);
}

TEST_CASE("Nanomsg request random test", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6671";

    // Create and start Nanomsg request server
    auto server = std::make_shared<TestRequestServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<TestRequestClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Create a new client and connect
        if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Nanomsg request client
                auto client = std::make_shared<TestRequestClient>(address);
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
        // Send a request from the random client
        else if ((rand() % 1) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsConnected())
                    client->Request("test");
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
