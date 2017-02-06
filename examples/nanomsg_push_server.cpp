/*!
    \file nanomsg_push_server.cpp
    \brief Nanomsg push server example
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/push_server.h"

#include <iostream>
#include <memory>

class ExamplePushServer : public CppServer::Nanomsg::PushServer
{
public:
    using CppServer::Nanomsg::PushServer::PushServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg push server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg push server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg push server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg push server address
    std::string address = "tcp://127.0.0.1:6666";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg push server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg push server
    auto server = std::make_shared<ExamplePushServer>(address);

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
    }

    // Stop the server
    server->Stop();

    return 0;
}
