/*!
    \file nanomsg_publisher_server.cpp
    \brief Nanomsg publisher server example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/publisher_server.h"

#include <iostream>
#include <memory>

class ExamplePublisherServer : public CppServer::Nanomsg::PublisherServer
{
public:
    using CppServer::Nanomsg::PublisherServer::PublisherServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg publisher server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg publisher server stopped!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg publisher server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg publisher server address
    std::string address = "tcp://*:6669";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg publisher server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg publisher server
    auto server = std::make_shared<ExamplePublisherServer>(address);

    // Start the server
    server->Start();

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
    server->Stop();

    return 0;
}
