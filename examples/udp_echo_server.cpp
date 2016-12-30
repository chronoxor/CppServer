/*!
    \file udp_echo_server.cpp
    \brief UDP echo server example
    \author Ivan Shynkarenka
    \date 22.12.2016
    \copyright MIT License
*/

#include "server/asio/udp_server.h"

#include <iostream>

class EchoServer : public CppServer::Asio::UDPServer
{
public:
    using CppServer::Asio::UDPServer::UDPServer;

protected:
    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::string messsage((const char*)buffer, size);
        std::cout << "Incoming: " << messsage << std::endl;

        // Echo the message back to the sender
        Send(endpoint, buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Echo UDP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP server port
    int port = 2222;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "UDP server port: " << port << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create a new UDP echo server
    auto server = std::make_shared<EchoServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

    // Start the server
    server->Start();

    // Wait for input
    std::cin.get();

    // Stop the server
    server->Stop();

    // Stop the service
    service->Stop();

    return 0;
}
