//
// Created by Ivan Shynkarenka on 03.02.2017
//

#include "catch.hpp"

#include "server/nanomsg/survey_client.h"
#include "server/nanomsg/survey_server.h"
#include "threads/thread.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <vector>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

class TestSurveyClient : public SurveyClient
{
public:
    std::atomic<bool> connected;
    std::atomic<bool> disconnected;
    std::atomic<bool> error;

    explicit TestSurveyClient(const std::string& address)
        : SurveyClient(address),
          connected(false),
          disconnected(false),
          error(false)
    {
    }

protected:
    void onConnected() override { connected = true; }
    void onDisconnected() override { disconnected = true; }
    void onReceived(Message& message) override { Send(message); }
    void onError(int error, const std::string& message) override { error = true; }
};

class TestSurveyServer : public SurveyServer
{
public:
    std::atomic<bool> started;
    std::atomic<bool> stopped;
    std::atomic<bool> error;

    explicit TestSurveyServer(const std::string& address)
        : SurveyServer(address),
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

TEST_CASE("Nanomsg survey server & client", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6674";

    // Create and start Nanomsg survey server
    auto server = std::make_shared<TestSurveyServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and connect Nanomsg survey client
    auto client = std::make_shared<TestSurveyClient>(address);
    REQUIRE(client->Connect());
    while (!client->IsConnected())
        Thread::Yield();

    // Sleep for a while...
    Thread::Sleep(1000);

    // Start the survey
    bool answers = false;
    server->Send("test");
    while (true)
    {
        Message msg;

        // Receive survey responses from clients
        std::tuple<size_t, bool> result = server->ReceiveSurvey(msg);

        // Show answers from survey clients
        if (std::get<0>(result) > 0)
            answers = true;

        // Finish the survey
        if (std::get<1>(result))
            break;
    }
    REQUIRE(answers);

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

TEST_CASE("Nanomsg survey random test", "[CppServer][Nanomsg]")
{
    const std::string address = "tcp://127.0.0.1:6675";

    // Create and start Nanomsg survey server
    auto server = std::make_shared<TestSurveyServer>(address);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Test duration in seconds
    const int duration = 10;

    // Clients collection
    std::vector<std::shared_ptr<TestSurveyClient>> clients;

    // Create and connect the first Nanomsg survey client
    auto client = std::make_shared<TestSurveyClient>(address);
    clients.emplace_back(client);
    client->Connect();
    while (!client->IsConnected())
        Thread::Yield();

    // Sleep for a while...
    Thread::Sleep(1000);

    // Start random test
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count() < duration)
    {
        // Create a new client and connect
        if ((rand() % 100) == 0)
        {
            if (clients.size() < 100)
            {
                // Create and connect Nanomsg survey client
                auto client = std::make_shared<TestSurveyClient>(address);
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
        // Start the survey
        else if ((rand() % 1000) == 0)
        {
            // Start the survey
            bool answers = false;
            server->Send("test");
            while (true)
            {
                Message msg;

                // Receive survey responses from clients
                std::tuple<size_t, bool> result = server->ReceiveSurvey(msg);

                // Show answers from survey clients
                if (std::get<0>(result) > 0)
                    answers = true;

                // Finish the survey
                if (std::get<1>(result))
                    break;
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
