/*!
    \file nanomsg_bus.cpp
    \brief Nanomsg bus example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus.h"

#include <iostream>
#include <memory>

class ExampleBus : public CppServer::Nanomsg::Bus
{
public:
    using CppServer::Nanomsg::Bus::Bus;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg bus node started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg bus node stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg bus node caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: nanomsg_bus address [node1] [node2]..." << std::endl;
        return -1;
    }

    // Nanomsg bus node address
    std::string address = argv[1];

    std::cout << "Nanomsg bus node address: " << address << std::endl;
    std::cout << "Press Enter to stop the bus node or '!' to restart the bus node..." << std::endl;

    // Create a new Nanomsg bus node
    auto server = std::make_shared<ExampleBus>(address);

    // Start the server
    server->Start();

    // Link the bus node to another bus nodes
    for (int i = 2; i < argc; ++i)
    {
        std::string link_address = argv[i];
        server->Link(link_address);
        std::cout << "Nanomsg bus node linked: " << link_address << std::endl;
    }

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

        // Send the entered text to all connected bus nodes
        server->Send(line);
    }

    // Stop the server
    server->Stop();

    return 0;
}
