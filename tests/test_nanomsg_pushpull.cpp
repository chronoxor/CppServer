//
// Created by Ivan Shynkarenka on 01.02.2017.
//

#include "catch.hpp"

#include "server/nanomsg/pull_server.h"
#include "server/nanomsg/push_client.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
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

class TestPullServer : public PullServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<size_t> received;
    std::atomic<bool> error;

    explicit TestPullServer(const std::string& address)
        : PullServer(address),
          started(false),
          stopped(false),
          received(0),
          error(false)
    {
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onReceived(Message& message) { received += message.size(); }
    void onError(int error, const std::string& message) override { error = true; }
};

TEST_CASE("Nanomsg pull server & push client", "[CppServer][Nanomsg]")
{
    const std::string server_address = "tcp://*:6666";
    const std::string client_address = "tcp://localhost:6666";

    // Create and start Nanomsg pull server
    TestPullServer server(server_address);
    REQUIRE(server.Start());
    while (!server.IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg push client
    TestPushClient client(client_address);
    REQUIRE(client.Connect());
    while (!client.IsConnected())
        Thread::Yield();

    // Push some data to the server
    client.Send("test");

    // Wait for all data processed...
    while (server.socket().bytes_received() != 4)
        Thread::Yield();

    // Disconnect the client
    REQUIRE(client.Disconnect());
    while (client.IsConnected())
        Thread::Yield();

    // Stop the server
    REQUIRE(server.Stop());
    while (server.IsStarted())
        Thread::Yield();

    // Check the server state
    REQUIRE(server.started);
    REQUIRE(server.stopped);
    REQUIRE(server.socket().established_connections() == 1);
    REQUIRE(server.socket().messages_received() == 1);
    REQUIRE(server.socket().bytes_received() == 4);
    REQUIRE(!server.error);

    // Check the client state
    REQUIRE(client.connected);
    REQUIRE(client.disconnected);
    REQUIRE(client.socket().established_connections() == 1);
    REQUIRE(client.socket().messages_sent() == 1);
    REQUIRE(client.socket().bytes_sent() == 4);
    REQUIRE(!client.error);
}

/*
TEST_CASE("TCP server random test", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 1113;

    // Create and start Asio service
    auto service = std::make_shared<EchoTCPService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start Echo server
    auto server = std::make_shared<EchoTCPServer>(service, InternetProtocol::IPv4, port);
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
            // Create and connect Echo client
            auto client = std::make_shared<EchoTCPClient>(service, address, port);
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
    REQUIRE(server->total_received() > 0);
    REQUIRE(server->total_sent() > 0);
    REQUIRE(!server->error);
}
*/
