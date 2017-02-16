/*!
    \file nanomsg_request_client.cpp
    \brief Nanomsg request client example
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/request_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExampleRequestClient : public CppServer::Nanomsg::RequestClient
{
public:
    using CppServer::Nanomsg::RequestClient::RequestClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg request client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg request client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg request client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg reply server address
    std::string address = "tcp://127.0.0.1:6668";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg reply server address: " << address << std::endl;

    // Create a new Nanomsg request client
    auto client = std::make_shared<ExampleRequestClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

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
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Request the entered text to the server
        CppServer::Nanomsg::Message message = client->Request(line);

        // Show the response message
        std::cout << "Response: " << message << std::endl;
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
