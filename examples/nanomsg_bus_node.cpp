/*!
    \file nanomsg_bus_node.cpp
    \brief Nanomsg bus node example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_node.h"

#include <iostream>
#include <memory>

class ExampleBusNode : public CppServer::Nanomsg::BusNode
{
public:
    using CppServer::Nanomsg::BusNode::BusNode;

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
        std::cout << "Usage: nanomsg_bus_node address [node1] [node2]..." << std::endl;
        return -1;
    }

    // Nanomsg bus node address
    std::string address = argv[1];

    std::cout << "Nanomsg bus node address: " << address << std::endl;
    std::cout << "Press Enter to stop the bus node or '!' to restart the bus node..." << std::endl;

    // Create a new Nanomsg bus node
    auto node = std::make_shared<ExampleBusNode>(address);

    // Start the bus node
    node->Start();

    // Link to another bus nodes
    for (int i = 2; i < argc; ++i)
    {
        std::string link_address = argv[i];
        node->Link(link_address);
        std::cout << "Nanomsg bus node linked: " << link_address << std::endl;
    }

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the bus node
        if (line == "!")
        {
            std::cout << "Node restarting...";
            node->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to all connected bus nodes
        node->Send(line);
    }

    // Stop the bus node
    node->Stop();

    return 0;
}
