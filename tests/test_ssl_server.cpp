//
// Created by Ivan Shynkarenka on 01.01.2016.
//
/*
#include "catch.hpp"

#include "server/asio/ssl_client.h"
#include "server/asio/ssl_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

class EchoSSLService : public Service
{
public:
    std::atomic<bool> thread_initialize;
    std::atomic<bool> thread_cleanup;
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> idle;
    std::atomic<bool> error;

    explicit EchoSSLService()
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

class EchoSSLClient : public SSLClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> handshaked;
    std::atomic<bool> disconnected;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoSSLClient(std::shared_ptr<EchoSSLService>& service, asio::ssl::context& context, const std::string& address, int port)
        : SSLClient(service, context, address, port),
          connected(false),
          handshaked(false),
          disconnected(false),
          received(0),
          sent(0),
          error(false)
    {
        stream().set_verify_mode(asio::ssl::verify_none);
    }

    static asio::ssl::context CreateContext()
    {
        asio::ssl::context context(asio::ssl::context::sslv23);
        return context;
    }

protected:
    void onConnected() override { connected = true; }
    void onHandshaked() override { handshaked = true; }
    void onDisconnected() override { disconnected = true; }
    size_t onReceived(const void* buffer, size_t size) override { received += size; return size; }
    void onSent(size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoSSLServer;

class EchoSSLSession : public SSLSession<EchoSSLServer, EchoSSLSession>
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> handshaked;
    std::atomic<bool> disconnected;
    std::atomic<size_t> received;
    std::atomic<size_t> sent;
    std::atomic<bool> error;

    explicit EchoSSLSession(asio::ip::tcp::socket socket, asio::ssl::context& context)
        : SSLSession<EchoSSLServer, EchoSSLSession>(std::move(socket), context),
          connected(false),
          handshaked(false),
          disconnected(false),
          received(0),
          sent(0),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onHandshaked() override { handshaked = true; }
    void onDisconnected() override { disconnected = true; }
    size_t onReceived(const void* buffer, size_t size) override { Send(buffer, size); received += size; return size; }
    void onSent(size_t sent, size_t pending) override { this->sent += sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

class EchoSSLServer : public SSLServer<EchoSSLServer, EchoSSLSession>
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

    explicit EchoSSLServer(std::shared_ptr<EchoSSLService> service, asio::ssl::context& context, InternetProtocol protocol, int port)
        : SSLServer<EchoSSLServer, EchoSSLSession>(service, context, protocol, port),
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

    static asio::ssl::context CreateContext()
    {
        asio::ssl::context context(asio::ssl::context::sslv23);
        context.set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
        context.set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "123qwe!"; });
        context.use_certificate_chain_file("../tools/certificates/server.pem");
        context.use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
        context.use_tmp_dh_file("../tools/certificates/dh2048.pem");
        return context;
    }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<EchoSSLSession> session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<EchoSSLSession> session) override { disconnected = true; --clients; received += session->received; sent += session->sent; }
    void onError(int error, const std::string& category, const std::string& message) override { error = true; }
};

TEST_CASE("SSL server & client", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 3333;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    asio::ssl::context server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    asio::ssl::context client_context = EchoSSLClient::CreateContext();

    // Create and connect Echo client
    auto client = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client->Connect());
    while (!client->IsConnected() || !client->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Send some data to the Echo server
    client->Send("test", 4);

    // Wait for all data processed...
    while (client->received != 4)
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client->Disconnect());
    while (client->IsConnected() || client->IsHandshaked() || (server->clients != 0))
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
    REQUIRE(client->handshaked);
    REQUIRE(client->disconnected);
    REQUIRE(client->received == 4);
    REQUIRE(client->sent == 4);
    REQUIRE(!client->error);
}

TEST_CASE("SSL server multicast", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 3334;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    asio::ssl::context server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    asio::ssl::context client_context = EchoSSLClient::CreateContext();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client1->Connect());
    while (!client1->IsConnected() || !client1->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while (client1->received != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client2->Connect());
    while (!client2->IsConnected() || !client2->IsHandshaked() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 8) || (client2->received != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client3->Connect());
    while (!client3->IsConnected() || !client3->IsHandshaked() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 8) || (client3->received != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->Disconnect());
    while (client1->IsConnected() || client1->IsHandshaked() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->Disconnect());
    while (client2->IsConnected() || client2->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test", 4);

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 12))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client3->Disconnect());
    while (client3->IsConnected() || client3->IsHandshaked() || (server->clients != 0))
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


TEST_CASE("SSL server random test", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 3335;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    asio::ssl::context server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Create and prepare a new SSL client context
    asio::ssl::context client_context = EchoSSLClient::CreateContext();

    // Clients collection
    std::vector<std::shared_ptr<EchoSSLClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Disconnect all clients
        if ((rand() % 1000) == 0)
        {
            server->DisconnectAll();
            clients.clear();
        }
        // Connect a new client
        else if ((rand() % 100) == 0)
        {
            // Create and connect Echo client
            auto client = std::make_shared<EchoSSLClient>(service, client_context, address, port);
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
        // Reconnect the random client
        else if ((rand() % 100) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                client->Reconnect();
            }
        }
        // Multicast a message to all clients
        else if ((rand() % 10) == 0)
        {
            server->Multicast("test", 4);
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
    REQUIRE(!server->error);
}
*/