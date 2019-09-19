//
// Created by Ivan Shynkarenka on 28.05.2019
//

#include "server/asio/service.h"
#include "server/ws/ws_server.h"
#include "system/cpu.h"

#include <iostream>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppServer::Asio;
using namespace CppServer::WS;

class EchoSession : public WSSession
{
public:
    using WSSession::WSSession;

protected:
    void onWSReceived(const void* buffer, size_t size) override
    {
        // Resend the message back to the client
        SendBinaryAsync(buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "WebSocket session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoServer : public WSServer
{
public:
    using WSServer::WSServer;

protected:
    std::shared_ptr<TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) override
    {
        return std::make_shared<EchoSession>(std::dynamic_pointer_cast<WSServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "WebSocket server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(8080).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").dest("threads").action("store").type("int").set_default(CPU::PhysicalCores()).help("Count of working threads. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        return 0;
    }

    // Server port
    int port = options.get("port");
    int threads = options.get("threads");

    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>(threads);

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, port);
    // server->SetupNoDelay(true);
    server->SetupReuseAddress(true);
    server->SetupReusePort(true);

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
