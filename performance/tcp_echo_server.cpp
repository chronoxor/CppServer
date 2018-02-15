//
// Created by Ivan Shynkarenka on 15.03.2017
//

#include "server/asio/service.h"
#include "server/asio/tcp_server.h"

#include <iostream>

#include <OptionParser.h>

using namespace CppServer::Asio;

class EchoSession;

class EchoServer : public TCPServer<EchoServer, EchoSession>
{
public:
    using TCPServer<EchoServer, EchoSession>::TCPServer;

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoSession : public TCPSession<EchoServer, EchoSession>
{
public:
    using TCPSession<EchoServer, EchoSession>::TCPSession;

protected:
    void onConnected() override
    {
        // Disable Nagle's algorithm
        socket().set_option(asio::ip::tcp::no_delay(true));
    }

    void onReceived(const void* buffer, size_t size) override
    {
        // Resend the message back to the client
        Send(buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-p", "--port").action("store").type("int").set_default(1111).help("Server port. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Server port
    int port = options.get("port");

    std::cout << "Server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, InternetProtocol::IPv4, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
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
