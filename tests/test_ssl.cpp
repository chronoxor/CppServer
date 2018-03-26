//
// Created by Ivan Shynkarenka on 01.01.2017
//

#include "test.h"

#include "server/asio/ssl_client.h"
#include "server/asio/ssl_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;

namespace {

class EchoSSLService : public Service
{
public:
    std::atomic<bool> thread_initialize;
    std::atomic<bool> thread_cleanup;
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> idle;
    std::atomic<bool> errors;

    EchoSSLService()
        : thread_initialize(false),
          thread_cleanup(false),
          started(false),
          stopped(false),
          idle(false),
          errors(false)
    {
    }

protected:
    void onThreadInitialize() override { thread_initialize = true; }
    void onThreadCleanup() override { thread_cleanup = true; }
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onIdle() override { idle = true; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }
};

class EchoSSLClient : public SSLClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> handshaked;
    std::atomic<bool> disconnected;
    std::atomic<bool> errors;

    EchoSSLClient(std::shared_ptr<EchoSSLService> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port)
        : SSLClient(service, context, address, port),
          connected(false),
          handshaked(false),
          disconnected(false),
          errors(false)
    {
    }

    static std::shared_ptr<asio::ssl::context> CreateContext()
    {
        auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
        context->set_verify_mode(asio::ssl::verify_peer);
        context->load_verify_file("../tools/certificates/ca.pem");
        return context;
    }

protected:
    void onConnected() override { connected = true; }
    void onHandshaked() override { handshaked = true; }
    void onDisconnected() override { disconnected = true; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }
};

class EchoSSLSession : public SSLSession
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> handshaked;
    std::atomic<bool> disconnected;
    std::atomic<bool> errors;

    EchoSSLSession(std::shared_ptr<SSLServer> server, std::shared_ptr<asio::ssl::context> context)
        : SSLSession(server, context),
          connected(false),
          handshaked(false),
          disconnected(false),
          errors(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onHandshaked() override { handshaked = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(const void* buffer, size_t size) override { Send(buffer, size); }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }
};

class EchoSSLServer : public SSLServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<size_t> clients;
    std::atomic<bool> errors;

    EchoSSLServer(std::shared_ptr<EchoSSLService> service, std::shared_ptr<asio::ssl::context> context, InternetProtocol protocol, int port)
        : SSLServer(service, context, protocol, port),
          started(false),
          stopped(false),
          connected(false),
          disconnected(false),
          clients(0),
          errors(false)
    {
    }

    static std::shared_ptr<asio::ssl::context> CreateContext()
    {
        auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::tlsv12);
        context->set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
        context->use_certificate_chain_file("../tools/certificates/server.pem");
        context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
        context->use_tmp_dh_file("../tools/certificates/dh4096.pem");
        return context;
    }

protected:
    std::shared_ptr<SSLSession> CreateSession(std::shared_ptr<SSLServer> server) override { return std::make_shared<EchoSSLSession>(server); }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<SSLSession>& session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<SSLSession>& session) override { disconnected = true; --clients; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }
};

} // namespace

TEST_CASE("SSL server", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 2222;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto client_context = EchoSSLServer::CreateContext();

    // Create and connect Echo client
    auto client = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client->Connect());
    while (!client->IsConnected() || !client->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Send a message to the Echo server
    client->Send("test");

    // Wait for all data processed...
    while (client->bytes_received() != 4)
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
    REQUIRE(!service->errors);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() == 4);
    REQUIRE(server->bytes_received() == 4);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client->connected);
    REQUIRE(client->handshaked);
    REQUIRE(client->disconnected);
    REQUIRE(client->bytes_sent() == 4);
    REQUIRE(client->bytes_received() == 4);
    REQUIRE(!client->errors);
}

TEST_CASE("SSL server multicast", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 2223;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto client_context = EchoSSLClient::CreateContext();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client1->Connect());
    while (!client1->IsConnected() || !client1->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while (client1->bytes_received() != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client2->Connect());
    while (!client2->IsConnected() || !client2->IsHandshaked() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 8) || (client2->bytes_received() != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoSSLClient>(service, client_context, address, port);
    REQUIRE(client3->Connect());
    while (!client3->IsConnected() || !client3->IsHandshaked() || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 8) || (client3->bytes_received() != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->Disconnect());
    while (client1->IsConnected() || client1->IsHandshaked() || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->Disconnect());
    while (client2->IsConnected() || client2->IsHandshaked() || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->Multicast("test");

    // Wait for all data processed...
    while ((client1->bytes_received() != 12) || (client2->bytes_received() != 12) || (client3->bytes_received() != 12))
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
    REQUIRE(!service->errors);

    // Check the Echo server state
    REQUIRE(server->started);
    REQUIRE(server->stopped);
    REQUIRE(server->connected);
    REQUIRE(server->disconnected);
    REQUIRE(server->bytes_sent() == 36);
    REQUIRE(server->bytes_received() == 0);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client1->bytes_sent() == 0);
    REQUIRE(client2->bytes_sent() == 0);
    REQUIRE(client3->bytes_sent() == 0);
    REQUIRE(client1->bytes_received() == 12);
    REQUIRE(client2->bytes_received() == 12);
    REQUIRE(client3->bytes_received() == 12);
    REQUIRE(!client1->errors);
    REQUIRE(!client2->errors);
    REQUIRE(!client3->errors);
}

TEST_CASE("SSL server random test", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 2224;

    // Create and start Asio service
    auto service = std::make_shared<EchoSSLService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoSSLServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoSSLServer>(service, server_context, InternetProtocol::IPv4, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Create and prepare a new SSL client context
    auto client_context = EchoSSLClient::CreateContext();

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
        }
        // Create a new client and connect
        else if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Echo client
                auto client = std::make_shared<EchoSSLClient>(service, client_context, address, port);
                clients.emplace_back(client);
                client->Connect();
                while (!client->IsHandshaked())
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
                if (client->IsHandshaked())
                {
                    client->Disconnect();
                    while (client->IsConnected())
                        Thread::Yield();
                }
                else if (!client->IsConnected())
                {
                    client->Connect();
                    while (!client->IsHandshaked())
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
                if (client->IsHandshaked())
                {
                    client->Reconnect();
                    while (!client->IsHandshaked())
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
                if (client->IsHandshaked())
                    client->Send("test");
            }
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Disconnect clients
    for (auto& client : clients)
    {
        client->Disconnect();
        while (client->IsConnected())
            Thread::Yield();
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
    REQUIRE(!server->errors);
}
