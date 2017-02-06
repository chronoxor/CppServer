/*!
    \file nanomsg_pair_client.cpp
    \brief Nanomsg pair client example
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/pair_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExamplePairClient : public CppServer::Nanomsg::PairClient
{
public:
    using CppServer::Nanomsg::PairClient::PairClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg pair client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg pair client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg pair client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg pair server address
    std::string address = "tcp://127.0.0.1:6667";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg pair server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg pair client
    auto client = std::make_shared<ExamplePairClient>(address);

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

        // Send the entered text to the server
        client->Send(line);
    }

    // Disconnect the client
    client->Disconnect();

    return 0;
}
