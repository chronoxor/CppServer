/*!
    \file rest_http_server.cpp
    \brief REST http server example
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/restbed/rest_server.h"

#include <iostream>
#include <memory>

class Storage
{
public:
    Storage(const std::string& data = "") : _data(data) {}

    void RestGet(const std::shared_ptr<restbed::Session> session)
    {
        std::cout << "GET /data: " << _data << std::endl;

        session->close(restbed::OK, _data, { { "Content-Length", std::to_string(_data.size()) } });
    }

    void RestPut(const std::shared_ptr<restbed::Session> session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);

        session->fetch(request_content_length, [this, request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            _data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /data: " << _data << std::endl;

            session->close(restbed::OK);
        });
    }

    void RestDelete(const std::shared_ptr<restbed::Session> session)
    {
        std::cout << "DELETE /data: " << _data << std::endl;

        _data = "";

        session->close(restbed::OK);
    }

    static void RestError(const int status_code, const std::exception error, const std::shared_ptr<restbed::Session> session)
    {
        if ((session != nullptr) && session->is_open())
        {
            std::string message = error.what();
            message.push_back('\n');

            session->close(status_code, message, { { "Content-Length", std::to_string(message.length()) } });
        }
    }

private:
    std::string _data;
};

int main(int argc, char** argv)
{
    // HTTP server port
    int port = 8000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "REST server port: " << port << std::endl;

    // Create a custom storage
    auto storage = std::make_shared<Storage>();

    // Create a resource for the storage
    auto resource = std::make_shared<restbed::Resource>();
    resource->set_path("/storage");
    resource->set_method_handler("GET", [storage](const std::shared_ptr<restbed::Session> session) { storage->RestGet(session); });
    resource->set_method_handler("PUT", [storage](const std::shared_ptr<restbed::Session> session) { storage->RestPut(session); });
    resource->set_method_handler("DELETE", [storage](const std::shared_ptr<restbed::Session> session) { storage->RestDelete(session); });

    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(port);
    settings->set_default_header("Connection", "close");

    // Create a Restbed service
    auto service = std::make_shared<restbed::Service>();
    service->publish(resource);
    service->set_error_handler(Storage::RestError);

    // Create a new HTTP server
    auto server = std::make_shared<CppServer::Restbed::RestServer>(service, settings);

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
