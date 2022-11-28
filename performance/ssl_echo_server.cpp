//
// Created by Ivan Shynkarenka on 16.03.2017
//

#include "server/asio/service.h"
#include "server/asio/ssl_server.h"
#include "system/cpu.h"

#include <iostream>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppServer::Asio;

class EchoSession : public SSLSession
{
public:
    using SSLSession::SSLSession;

protected:
    void onReceived(const void* buffer, size_t size) override
    {
        // Resend the message back to the client
        SendAsync(buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoServer : public SSLServer
{
public:
    using SSLServer::SSLServer;

protected:
    std::shared_ptr<SSLSession> CreateSession(const std::shared_ptr<SSLServer>& server) override
    {
        return std::make_shared<EchoSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "SSL server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(2222).help("Server port. Default: %default");
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

    // Create and prepare a new SSL server context
    auto context = std::make_shared<SSLContext>(asio::ssl::context::tlsv13);
    context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, context, port);
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
