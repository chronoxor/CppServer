/*!
    \file nanomsg_pull_server.cpp
    \brief Nanomsg pull server example
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/server.h"

#include <iostream>

class PullServer : public CppServer::Nanomsg::Server
{
public:
    using CppServer::Nanomsg::Server::Server;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg pull server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg pull server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& msg)
    {
        std::string message((const char*)msg.buffer(), msg.size());
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg pull server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg pull server address
    std::string address = "tcp://*:10000";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg pull server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg pull server
    PullServer server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Pull, address);

    // Start the server in a separate thread
    server.StartThread();

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
            server.Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    server.Stop();

    return 0;
}
