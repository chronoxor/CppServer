//
// Created by Ivan Shynkarenka on 16.03.2017
//

#include "server/asio/service.h"
#include "server/asio/web_server.h"

#include <atomic>
#include <iostream>

#include "../../modules/cpp-optparse/OptionParser.h"

using namespace CppServer::Asio;

class EchoServer : public WebServer
{
public:
    explicit EchoServer(std::shared_ptr<Service> service, int port)
        : WebServer(service, port)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage/{key: .*}");
        resource->set_method_handler("POST", RestStoragePost);

        // Publish the resource
        server()->publish(resource);
    }

private:
    static void RestStoragePost(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
        });
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-p", "--port").action("store").type("int").set_default(8000).help("Server port. Default: %default");

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

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, port);

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

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
