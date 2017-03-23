//
// Created by Ivan Shynkarenka on 23.03.2017
//

#include "server/nanomsg/request_server.h"

#include <iostream>

#include "../../modules/cpp-optparse/OptionParser.h"

using namespace CppServer::Nanomsg;

class EchoServer : public RequestServer
{
public:
    using RequestServer::RequestServer;

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        Send(message);
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("tcp://127.0.0.1:6666").help("Server address. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Server address
    std::string address(options.get("address"));

    std::cout << "Server address: " << address << std::endl;

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(address);

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

    return 0;
}
