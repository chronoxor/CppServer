//
// Created by Ivan Shynkarenka on 07.01.2022
//

#include "server/asio/service.h"
#include "server/asio/tcp_server.h"
#include "system/cpu.h"

#include "../proto/simple_protocol.h"

#include <iostream>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppServer::Asio;

class ProtoSession : public TCPSession, public FBE::simple::Sender, public FBE::simple::Receiver
{
public:
    using TCPSession::TCPSession;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Protocol session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

protected:
    // Protocol handlers
    void onReceive(const ::simple::SimpleRequest& request) override
    {
        // Send response
        simple::SimpleResponse response;
        response.id = request.id;
        response.Hash = 0;
        response.Length = (uint32_t)request.Message.size();
        send(response);
    }

    // Protocol implementation
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }
};

class ProtoServer : public TCPServer, public FBE::simple::Sender
{
public:
    using TCPServer::TCPServer;

protected:
    std::shared_ptr<TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) override
    {
        return std::make_shared<ProtoSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Protocol server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

protected:
    // Protocol implementation
    size_t onSend(const void* data, size_t size) override { Multicast(data, size); return size; }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(4444).help("Server port. Default: %default");
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

    // Create a new protocol server
    auto server = std::make_shared<ProtoServer>(service, port);
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
