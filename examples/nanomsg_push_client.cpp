/*!
    \file nanomsg_push_client.cpp
    \brief Nanomsg push client example
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/push_client.h"
#include "threads/thread.h"

#include <iostream>

class PushClient : public CppServer::Nanomsg::PushClient
{
public:
    using CppServer::Nanomsg::PushClient::PushClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg push client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg push client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg push client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg push server address
    std::string address = "tcp://localhost:6666";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg push server address: " << address << std::endl;
    std::cout << "Press Enter to stop or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg push client
    PushClient client(address);

    // Start the client
    client.Connect();

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
            client.Disconnect();
            continue;
        }

        // Send the entered text to the server
        client.Send(line);
    }

    // Disconnect the client
    client.Disconnect();

    return 0;
}
