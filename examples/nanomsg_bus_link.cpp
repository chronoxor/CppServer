/*!
    \file nanomsg_bus_link.cpp
    \brief Nanomsg bus link example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_link.h"

#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: nanomsg_bus_link node1 node2 [node3]..." << std::endl;
        return -1;
    }

    // Nanomsg bus link addresses
    std::string address1 = argv[1];
    std::string address2 = argv[2];

    std::cout << "Nanomsg link a bus node " << address1 << " with a bus node " << address2 << std::endl;
    std::cout << "Press Enter to stop the bus link or '!' to restart the bus link..." << std::endl;

    // Create a new Nanomsg bus link
    auto link = std::make_shared<CppServer::Nanomsg::BusLink>(address1, address2);

    // Link to another bus nodes
    for (int i = 3; i < argc; ++i)
    {
        std::string link_address = argv[i];
        link->Link(link_address);
        std::cout << "Nanomsg bus node linked: " << link_address << std::endl;
    }

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the link
        if (line == "!")
        {
            std::cout << "Link restarting...";
            link->Reconnect();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to all connected bus nodes
        link->Send(line);
    }

    return 0;
}
