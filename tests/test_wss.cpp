//
// Created by Ivan Shynkarenka on 29.05.2019
//

#include "test.h"

#include "server/ws/wss_client.h"
#include "server/ws/wss_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Asio;
using namespace CppServer::WS;

namespace {

class EchoWSSService : public Service
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

class EchoWSSClient : public WSSClient
{
public:
    using WSSClient::WSSClient;

    static std::shared_ptr<SSLContext> CreateContext()
    {
        auto context = std::make_shared<SSLContext>(asio::ssl::context::tlsv13);
        context->set_default_verify_paths();
        context->set_root_certs();
        context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        context->load_verify_file("../tools/certificates/ca.pem");
        return context;
    }

protected:
    void onWSConnecting(CppServer::HTTP::HTTPRequest& request) override
    {
        request.SetBegin("GET", "/");
        request.SetHeader("Host", "localhost");
        request.SetHeader("Origin", "https://localhost");
        request.SetHeader("Upgrade", "websocket");
        request.SetHeader("Connection", "Upgrade");
        request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(ws_nonce()));
        request.SetHeader("Sec-WebSocket-Protocol", "chat, superchat");
        request.SetHeader("Sec-WebSocket-Version", "13");
    }
    void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override { connected = true; }
    void onWSDisconnected() override { disconnected = true; }
    void onWSReceived(const void* buffer, size_t size) override { received += size; }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<size_t> received{0};
    std::atomic<bool> errors{false};
};

class EchoWSSSession : public WSSSession
{
public:
    using WSSSession::WSSSession;

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override { connected = true; }
    void onWSDisconnected() override { disconnected = true; }
    void onWSReceived(const void* buffer, size_t size) override { SendBinaryAsync(buffer, size); }
    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};
    std::atomic<bool> errors{false};
};

class EchoWSSServer : public WSSServer
{
public:
    using WSSServer::WSSServer;

    static std::shared_ptr<SSLContext> CreateContext()
    {
        auto context = std::make_shared<SSLContext>(asio::ssl::context::tlsv13);
        context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
        context->use_certificate_chain_file("../tools/certificates/server.pem");
        context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
        context->use_tmp_dh_file("../tools/certificates/dh4096.pem");
        return context;
    }

protected:
    std::shared_ptr<SSLSession> CreateSession(const std::shared_ptr<SSLServer>& server) override { return std::make_shared<EchoWSSSession>(std::dynamic_pointer_cast<WSSServer>(server)); }

protected:
    void onStarted() override { started = true; }
    void onStopped() override { stopped = true; }
    void onConnected(std::shared_ptr<SSLSession>& session) override { connected = true; ++clients; }
    void onDisconnected(std::shared_ptr<SSLSession>& session) override { disconnected = true; --clients; }
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

TEST_CASE("WebSocket secure server test", "[CppServer][WebSocket]")
{
    const std::string address = "127.0.0.1";
    const int port = 8444;

    // Create and start Asio service
    auto service = std::make_shared<EchoWSSService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoWSSServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoWSSServer>(service, server_context, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto client_context = EchoWSSServer::CreateContext();

    // Create and connect Echo client
    auto client = std::make_shared<EchoWSSClient>(service, client_context, address, port);
    REQUIRE(client->ConnectAsync());
    while (!client->connected || (server->clients != 1))
        Thread::Yield();

    // Send a message to the Echo server
    client->SendTextAsync("test");

    // Wait for all data processed...
    while (client->received != 4)
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client->CloseAsync(1000));
    while (!client->disconnected || (server->clients != 0))
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
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client->connected);
    REQUIRE(client->disconnected);
    REQUIRE(client->bytes_sent() > 0);
    REQUIRE(client->bytes_received() > 0);
    REQUIRE(!client->errors);
}

TEST_CASE("WebSocket secure server multicast test", "[CppServer][WebSocket]")
{
    const std::string address = "127.0.0.1";
    const int port = 8445;

    // Create and start Asio service
    auto service = std::make_shared<EchoWSSService>();
    REQUIRE(service->Start(true));
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoWSSServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoWSSServer>(service, server_context, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto client_context = EchoWSSServer::CreateContext();

    // Create and connect Echo client
    auto client1 = std::make_shared<EchoWSSClient>(service, client_context, address, port);
    REQUIRE(client1->ConnectAsync());
    while (!client1->connected || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->MulticastText("test");

    // Wait for all data processed...
    while (client1->received != 4)
        Thread::Yield();

    // Create and connect Echo client
    auto client2 = std::make_shared<EchoWSSClient>(service, client_context, address, port);
    REQUIRE(client2->ConnectAsync());
    while (!client2->connected || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->MulticastText("test");

    // Wait for all data processed...
    while ((client1->received != 8) || (client2->received != 4))
        Thread::Yield();

    // Create and connect Echo client
    auto client3 = std::make_shared<EchoWSSClient>(service, client_context, address, port);
    REQUIRE(client3->ConnectAsync());
    while (!client3->connected || (server->clients != 3))
        Thread::Yield();

    // Multicast some data to all clients
    server->MulticastText("test");

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 8) || (client3->received != 4))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client1->CloseAsync(1000));
    while (!client1->disconnected || (server->clients != 2))
        Thread::Yield();

    // Multicast some data to all clients
    server->MulticastText("test");

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 8))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client2->CloseAsync(1000));
    while (!client2->disconnected || (server->clients != 1))
        Thread::Yield();

    // Multicast some data to all clients
    server->MulticastText("test");

    // Wait for all data processed...
    while ((client1->received != 12) || (client2->received != 12) || (client3->received != 12))
        Thread::Yield();

    // Disconnect the Echo client
    REQUIRE(client3->CloseAsync(1000));
    while (!client3->disconnected || (server->clients != 0))
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
    REQUIRE(server->bytes_sent() > 0);
    REQUIRE(server->bytes_received() > 0);
    REQUIRE(!server->errors);

    // Check the Echo client state
    REQUIRE(client1->bytes_sent() > 0);
    REQUIRE(client2->bytes_sent() > 0);
    REQUIRE(client3->bytes_sent() > 0);
    REQUIRE(client1->bytes_received() > 0);
    REQUIRE(client2->bytes_received() > 0);
    REQUIRE(client3->bytes_received() > 0);
    REQUIRE(!client1->errors);
    REQUIRE(!client2->errors);
    REQUIRE(!client3->errors);
}

TEST_CASE("WebSocket secure server random test", "[CppServer][WebSocket]")
{
    const std::string address = "127.0.0.1";
    const int port = 8446;

    // Create and start Asio service
    auto service = std::make_shared<EchoWSSService>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = EchoWSSServer::CreateContext();

    // Create and start Echo server
    auto server = std::make_shared<EchoWSSServer>(service, server_context, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Create and prepare a new SSL client context
    auto client_context = EchoWSSServer::CreateContext();

    // Clients collection
    std::vector<std::shared_ptr<EchoWSSClient>> clients;

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Disconnect all clients
        if ((rand() % 1000) == 0)
        {
            server->CloseAll(1000);
        }
        // Create a new client and connect
        else if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Echo client
                auto client = std::make_shared<EchoWSSClient>(service, client_context, address, port);
                clients.emplace_back(client);
                client->ConnectAsync();
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
                    client->CloseAsync(1000);
                    while (client->IsConnected())
                        Thread::Yield();
                }
                else if (!client->IsConnected())
                {
                    client->ConnectAsync();
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
                    client->ReconnectAsync();
                    while (!client->IsHandshaked())
                        Thread::Yield();
                }
            }
        }
        // Multicast a message to all clients
        else if ((rand() % 10) == 0)
        {
            server->MulticastText("test");
        }
        // Send a message from the random client
        else if ((rand() % 1) == 0)
        {
            if (!clients.empty())
            {
                size_t index = rand() % clients.size();
                auto client = clients.at(index);
                if (client->IsHandshaked())
                    client->SendTextAsync("test");
            }
        }

        // Sleep for a while...
        Thread::Sleep(1);
    }

    // Disconnect clients
    for (auto& client : clients)
    {
        client->CloseAsync(1000);
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
