/*!
    \file rest_http_server.cpp
    \brief REST HTTP server example
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/rest_server.h"

#include <iostream>
#include <memory>

class Storage
{
public:
    Storage(const std::string& data = "") : _data(data) {}

    const std::string& GetData() const { return _data; }
    void SetData(const std::string& value) { _data = value; }

private:
    std::string _data;
};

class RestServer : public CppServer::Asio::RestServer
{
public:
    explicit RestServer(std::shared_ptr<CppServer::Asio::Service> service, int port)
        : CppServer::Asio::RestServer(service, port)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage");
        resource->set_method_handler("GET", [this](const std::shared_ptr<restbed::Session> session) { RestStorageGet(session); });
        resource->set_method_handler("PUT", [this](const std::shared_ptr<restbed::Session> session) { RestStoragePut(session); });
        resource->set_method_handler("DELETE", [this](const std::shared_ptr<restbed::Session> session) { RestStorageDelete(session); });

        // Publish the resource
        server()->publish(resource);
    }

private:
    Storage _storage;

    void RestStorageGet(const std::shared_ptr<restbed::Session> session)
    {
        std::string data = _storage.GetData();

        std::cout << "GET /storage: " << data << std::endl;

        session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
    }

    void RestStoragePut(const std::shared_ptr<restbed::Session> session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);

        session->fetch(request_content_length, [this, request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /storage: " << data << std::endl;

            _storage.SetData(data);

            session->close(restbed::OK);
        });
    }

    void RestStorageDelete(const std::shared_ptr<restbed::Session> session)
    {
        std::cout << "DELETE /storage" << std::endl;

        _storage.SetData("");

        session->close(restbed::OK);
    }

    static void RestErrorHandler(const int status_code, const std::exception error, const std::shared_ptr<restbed::Session> session)
    {
        if ((session != nullptr) && session->is_open())
        {
            std::string message = error.what();
            message.push_back('\n');

            session->close(status_code, message, { { "Content-Length", std::to_string(message.length()) } });
        }
    }
};

int main(int argc, char** argv)
{
    // REST HTTP server port
    int port = 8000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "REST HTTP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new REST HTTP server
    auto server = std::make_shared<RestServer>(service, port);

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
