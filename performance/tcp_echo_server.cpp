#include "server/asio/service.h"
#include "server/asio/tcp_server.h"

#include <atomic>
#include <iostream>

#include "../../modules/cpp-optparse/OptionParser.h"

std::atomic<size_t> errors(0);
std::atomic<size_t> connected(0);
std::atomic<size_t> transfered(0);

class EchoSession;

class EchoServer : public CppServer::Asio::TCPServer<EchoServer, EchoSession>
{
public:
    using CppServer::Asio::TCPServer<EchoServer, EchoSession>::TCPServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        ++errors;
    }
};

class EchoSession : public CppServer::Asio::TCPSession<EchoServer, EchoSession>
{
public:
    using CppServer::Asio::TCPSession<EchoServer, EchoSession>::TCPSession;

protected:
    void onConnected() override
    {
        ++connected;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        // Resend the message back to the client
        Send(buffer, size);

        // Update transfered statistics
        transfered += size;

        // Inform that we handled the whole buffer
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        ++errors;
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
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

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

    // Stop the service
    service->Stop();

    std::cout << "Server statistics: " << std::endl;
    std::cout << "- Caught errors: " << errors << std::endl;
    std::cout << "- Connected clients: " << connected << std::endl;
    std::cout << "- Bytes transfered: " << transfered << std::endl;

    return 0;
}
