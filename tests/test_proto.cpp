//
// Created by Ivan Shynkarenka on 07.01.2022
//

#include "test.h"

#include "server/asio/tcp_client.h"
#include "server/asio/tcp_server.h"
#include "threads/thread.h"

#include "../proto/simple_protocol.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

namespace {

class ProtoService : public Service
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

class ProtoClient : public TCPClient, public FBE::simple::Client
{
public:
    using TCPClient::TCPClient;

protected:
    void onConnected() override { reset(); connected = true; }
    void onDisconnected() override { disconnected = true; }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errors{false};
};

class ProtoSession : public TCPSession, public FBE::simple::Sender, public FBE::simple::Receiver
{
public:
    using TCPSession::TCPSession;

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

protected:
    // Protocol handlers
    void onReceive(const ::simple::SimpleRequest& request) override
    {
        // Send response
        simple::SimpleResponse response;
        response.id = request.id;
        response.Hash = 0;
        response.Length = (uint32_t)request.Message.size();
        send(response);
    }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errors{false};
};

class ProtoServer : public TCPServer, public FBE::simple::Sender
{
public:
    using TCPServer::TCPServer;

protected:
    std::shared_ptr<TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) override { return std::make_shared<ProtoSession>(server); }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<TCPSession>& session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<TCPSession>& session) override { disconnected = true; --clients; }
    size_t onSend(const void* data, size_t size) override { Multicast(data, size); return size; }
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

TEST_CASE("Protocol server test", "[CppServer][Proto]")
{
    const std::string address = "127.0.0.1";
    const int port = 4444;

    // Create and start Asio service
    auto service = std::make_shared<ProtoService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start protocol server
    auto server = std::make_shared<ProtoServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect protocol client
    auto client = std::make_shared<ProtoClient>(service, address, port);
    REQUIRE(client->ConnectAsync());
    while (!client->IsConnected() || !client->connected || (server->clients != 1))
        Thread::Yield();

    // Send a request to the protocol server
    simple::SimpleRequest request;
    request.Message = "test";
    auto response = client->request(request).get();
    REQUIRE(response.id == request.id);
    REQUIRE(response.Hash == 0);
    REQUIRE(response.Length == 4);

    // Disconnect the protocol client
    REQUIRE(client->DisconnectAsync());
    while (client->IsConnected() || (server->clients != 0))
        Thread::Yield();

    // Stop the protocol server
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

    // Check the protocol server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->errors);

    // Check the protocol client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->bytes_sent() > 0);
    REQUIRE(client->bytes_received() > 0);
    REQUIRE(!client->errors);
}

TEST_CASE("Protocol multicast test", "[CppServer][Proto]")
{
    const std::string address = "127.0.0.1";
    const int port = 4442;

    // Create and start Asio service
    auto service = std::make_shared<ProtoService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start protocol server
    auto server = std::make_shared<ProtoServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect protocol client
    auto client1 = std::make_shared<ProtoClient>(service, address, port);
    REQUIRE(client1->ConnectAsync());
    while (!client1->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Create a server notification
    simple::SimpleNotify notify;
    notify.Notification = "test";

    // Multicast the notification to all clients
    server->send(notify);

    // Wait for all data processed...
    while (client1->bytes_received() == 0)
        Thread::Yield();

    // Create and connect protocol client
    auto client2 = std::make_shared<ProtoClient>(service, address, port);
    REQUIRE(client2->ConnectAsync());
    while (!client2->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Multicast the notification to all clients
    server->send(notify);

    // Wait for all data processed...
    while (client2->bytes_received() == 0)
        Thread::Yield();

    // Create and connect protocol client
    auto client3 = std::make_shared<ProtoClient>(service, address, port);
    REQUIRE(client3->ConnectAsync());
    while (!client3->IsConnected() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->send(notify);

    // Wait for all data processed...
    while (client3->bytes_received() == 0)
        Thread::Yield();

    // Disconnect the protocol client
    REQUIRE(client1->DisconnectAsync());
    while (client1->IsConnected() || (server->clients != 2))
        Thread::Yield();

    // Disconnect the protocol client
    REQUIRE(client2->DisconnectAsync());
    while (client2->IsConnected() || (server->clients != 1))
        Thread::Yield();

    // Disconnect the protocol client
    REQUIRE(client3->DisconnectAsync());
    while (client3->IsConnected() || (server->clients != 0))
        Thread::Yield();

    // Stop the protocol server
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

    // Check the protocol server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() == 0);
    REQUIRE(!server->errors);

    // Check the protocol client state
    REQUIRE(client1->bytes_sent() == 0);
    REQUIRE(client2->bytes_sent() == 0);
    REQUIRE(client3->bytes_sent() == 0);
    REQUIRE(client1->bytes_received() > 0);
    REQUIRE(client2->bytes_received() > 0);
    REQUIRE(client3->bytes_received() > 0);
    REQUIRE(!client1->errors);
    REQUIRE(!client2->errors);
    REQUIRE(!client3->errors);
}

TEST_CASE("Protocol server random test", "[CppServer][Proto]")
{
    const std::string address = "127.0.0.1";
    const int port = 4443;

    // Create and start Asio service
    auto service = std::make_shared<ProtoService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start protocol server
    auto server = std::make_shared<ProtoServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<ProtoClient>> clients;

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
                // Create and connect protocol client
                auto client = std::make_shared<ProtoClient>(service, address, port);
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
        // Multicast a notification to all clients
        else if ((rand() % 10) == 0)
        {
            simple::SimpleNotify notify;
            notify.Notification = "test";
            server->send(notify);
        }
        // Send a request from the random client
        else if ((rand() % 1) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsConnected() && client->connected)
                {
                    simple::SimpleRequest request;
                    request.Message = "test";
                    client->request(request);
                }
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

    // Stop the protocol server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the protocol server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->errors);
}
