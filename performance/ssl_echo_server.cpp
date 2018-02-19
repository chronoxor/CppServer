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

class EchoSession;

class EchoServer : public SSLServer<EchoServer, EchoSession>
{
public:
    using SSLServer<EchoServer, EchoSession>::SSLServer;

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoSession : public SSLSession<EchoServer, EchoSession>
{
public:
    using SSLSession<EchoServer, EchoSession>::SSLSession;

protected:
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
    parser.add_option("-p", "--port").action("store").type("int").set_default(2222).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CPU::LogicalCores()).help("Count of working threads. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Server port
    int port = options.get("port");
    int threads_count = options.get("threads");

    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start(false, threads_count);
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    auto context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    context->set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    context->set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, context, InternetProtocol::IPv4, port);
    server->SetupNoDelay(true);
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
