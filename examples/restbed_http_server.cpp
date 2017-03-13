/*!
    \file restbed_http_server.cpp
    \brief Restbed HTTP server example
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/restbed/restbed.h"

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

class RestServer : public std::enable_shared_from_this<RestServer>
{
public:
    RestServer(int port)
    {
        // Create a Restbed resource
        _resource = std::make_shared<restbed::Resource>();
        _resource->set_path("/storage");
        _resource->set_method_handler("GET", [this](const std::shared_ptr<restbed::Session> session) { RestStorageGet(session); });
        _resource->set_method_handler("PUT", [this](const std::shared_ptr<restbed::Session> session) { RestStoragePut(session); });
        _resource->set_method_handler("DELETE", [this](const std::shared_ptr<restbed::Session> session) { RestStorageDelete(session); });

        // Create a Restbed settings
        _settings = std::make_shared<restbed::Settings>();
        _settings->set_port(port);
        _settings->set_default_header("Connection", "close");

        // Create a Restbed service
        _service = std::make_shared<restbed::Service>();
        _service->publish(_resource);
        _service->set_error_handler(RestErrorHandler);
    }

    void Start()
    {
        _service->start(_settings);
    }

    void Stop()
    {
        _service->stop();
    }

    void Restart()
    {
        _service->restart(_settings);
    }

private:
    Storage _storage;
    std::shared_ptr<restbed::Resource> _resource;
    std::shared_ptr<restbed::Settings> _settings;
    std::shared_ptr<restbed::Service> _service;

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
    // HTTP server port
    int port = 8000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "REST server port: " << port << std::endl;

    // Create a new Restbed HTTP server
    auto server = std::make_shared<RestServer>(port);

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
