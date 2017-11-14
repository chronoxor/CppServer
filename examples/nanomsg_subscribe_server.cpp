/*!
    \file nanomsg_subscribe_server.cpp
    \brief Nanomsg subscribe server example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/subscribe_server.h"

#include <iostream>
#include <memory>

class ExampleSubscribeServer : public CppServer::Nanomsg::SubscribeServer
{
public:
    using CppServer::Nanomsg::SubscribeServer::SubscribeServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg subscribe server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg subscribe server stopped!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg subscribe server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg subscribe server address
    std::string address = "tcp://127.0.0.1:6669";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg subscribe server address: " << address << std::endl;

    // Create a new Nanomsg subscribe server
    auto server = std::make_shared<ExampleSubscribeServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Publish the entered text to all subscribed clients
        server->Send(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
