#include "server/asio/service.h"
#include "server/asio/tcp_server.h"

#include <iostream>

#include "../../modules/cpp-optparse/OptionParser.h"

class EchoSession;

class EchoServer : public CppServer::Asio::TCPServer<EchoServer, EchoSession>
{
public:
    using CppServer::Asio::TCPServer<EchoServer, EchoSession>::TCPServer;
};

class EchoSession : public CppServer::Asio::TCPSession<EchoServer, EchoSession>
{
public:
    using CppServer::Asio::TCPSession<EchoServer, EchoSession>::TCPSession;

protected:
    size_t onReceived(const void* buffer, size_t size) override
    {
        // Resend the message back to the client
        Send(buffer, size);

        // Inform that we handled the whole buffer
        return size;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version(version);

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-p", "--port").action("store").type("int").set_default(1111).help("Server port. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // TCP server port
    int port = options.get("port");

    std::cout << "TCP server port: " << port << std::endl;
    std::cout << "Press Enter to stop the server..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create a new TCP echo server
    auto server = std::make_shared<EchoServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

    // Start the server
    server->Start();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;
    }

    // Stop the server
    server->Stop();

    // Stop the service
    service->Stop();

    return 0;
}
