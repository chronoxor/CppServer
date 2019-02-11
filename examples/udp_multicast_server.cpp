/*!
    \file udp_multicast_server.cpp
    \brief UDP multicast server example
    \author Ivan Shynkarenka
    \date 27.12.2016
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/udp_server.h"

#include <iostream>

class MulticastServer : public CppServer::Asio::UDPServer
{
public:
    using CppServer::Asio::UDPServer::UDPServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Multicast UDP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP multicast address
    std::string multicast_address = "239.255.0.1";
    if (argc > 1)
        multicast_address = argv[1];

    // UDP multicast port
    int multicast_port = 3334;
    if (argc > 2)
        multicast_port = std::atoi(argv[2]);

    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast server
    auto server = std::make_shared<MulticastServer>(service, 0);

    // Start the multicast server
    std::cout << "Server starting...";
    server->Start(multicast_address, multicast_port);
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

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
