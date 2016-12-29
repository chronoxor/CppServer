/*!
    \file udp_multicast_server.cpp
    \brief UDP multicast server example
    \author Ivan Shynkarenka
    \date 27.12.2016
    \copyright MIT License
*/

#include "server/asio/udp_server.h"

#include <iostream>

class MulticastServer : public CppServer::Asio::UDPServer
{
public:
    using CppServer::Asio::UDPServer::UDPServer;

protected:
    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::string messsage((const char*)buffer, size);
        std::cout << "Incoming: " << messsage << std::endl;

        // Multicast message
        Multicast(buffer, size);
    }

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
    int multicast_port = 1236;
    if (argc > 2)
        multicast_port = std::atoi(argv[2]);

    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create a new UDP multicast server
    auto server = std::make_shared<MulticastServer>(service, CppServer::Asio::InternetProtocol::IPv4, 0);

    // Start the multicast server
    server->Start(multicast_address, multicast_port);

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line.data(), line.size());
    }

    // Stop the server
    server->Stop();

    // Stop the service
    service->Stop();

    return 0;
}
