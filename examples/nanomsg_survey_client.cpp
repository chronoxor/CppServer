/*!
    \file nanomsg_survey_client.cpp
    \brief Nanomsg survey client example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/survey_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExampleSurveyClient : public CppServer::Nanomsg::SurveyClient
{
public:
    using CppServer::Nanomsg::SurveyClient::SurveyClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg survey client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg survey client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Nanomsg::Message& msg) override
    {
        std::string message((const char*)msg.buffer(), msg.size());
        std::cout << "Incoming: " << message << std::endl;

        // Send the reversed respond message back to the server
        Send(std::string(message.rbegin(), message.rend()));
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg survey client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg surveyor server address
    std::string address = "tcp://localhost:6670";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg surveyor server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg survey client
    auto client = std::make_shared<ExampleSurveyClient>(address);

    // Start the client
    client->Connect();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->Disconnect();
            continue;
        }
    }

    // Disconnect the client
    client->Disconnect();

    return 0;
}
