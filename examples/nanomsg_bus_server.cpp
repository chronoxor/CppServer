/*!
    \file nanomsg_bus_server.cpp
    \brief Nanomsg bus server example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_server.h"

#include <iostream>
#include <memory>

class ExampleBusServer : public CppServer::Nanomsg::BusServer
{
public:
    using CppServer::Nanomsg::BusServer::BusServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg bus server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg bus server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& msg) override
    {
        std::string message((const char*)msg.buffer(), msg.size());
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg bus server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: nanomsg_bus_server server [client1] [client2]..." << std::endl;
        return -1;
    }

    // Nanomsg pair bus address
    std::string address = argv[1];

    std::cout << "Nanomsg bus server address: " << address << std::endl;
    std::cout << "Press Enter to connect bus server, then to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg bus server
    auto server = std::make_shared<ExampleBusServer>(address);

    // Start the server
    server->Start();

    // Wait for user input to connect
    std::string line;
    getline(std::cin, line);

    // Connect to another servers
    for (int i = 2; i < argc; ++i)
    {
        std::string connect_address = argv[i];
        server->Connect(connect_address);
        std::cout << "Nanomsg bus server connected: " << connect_address << std::endl;
    }

    // Perform text input
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

        // Send the entered text to all connected bus servers
        server->Send(line);
    }

    // Stop the server
    server->Stop();

    return 0;
}
