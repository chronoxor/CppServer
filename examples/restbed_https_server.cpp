/*!
    \file rest_https_server.cpp
    \brief REST HTTPS server example
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/rest_server.h"

#include <iostream>
#include <memory>
#include <map>

class RestServerSSL : public CppServer::Asio::RestServer
{
public:
    explicit RestServerSSL(std::shared_ptr<CppServer::Asio::Service> service, int port)
        : CppServer::Asio::RestServer(service, port)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage/{key: .*}");
        resource->set_method_handler("POST", [this](const std::shared_ptr<restbed::Session> session) { RestStoragePost(session); });
        resource->set_method_handler("GET", [this](const std::shared_ptr<restbed::Session> session) { RestStorageGet(session); });
        resource->set_method_handler("PUT", [this](const std::shared_ptr<restbed::Session> session) { RestStoragePut(session); });
        resource->set_method_handler("DELETE", [this](const std::shared_ptr<restbed::Session> session) { RestStorageDelete(session); });

        // Publish the resource
        server()->publish(resource);

        // Prepare SSL settings
        ssl_settings()->set_http_disabled(true);
        ssl_settings()->set_default_workarounds_enabled(true);
        ssl_settings()->set_sslv2_enabled(false);
        ssl_settings()->set_single_diffie_hellman_use_enabled(true);
        ssl_settings()->set_passphrase("qwerty");
        ssl_settings()->set_certificate_chain(restbed::Uri("file://../tools/certificates/server.pem"));
        ssl_settings()->set_private_key(restbed::Uri("file://../tools/certificates/server.pem"));
        ssl_settings()->set_temporary_diffie_hellman(restbed::Uri("file://../tools/certificates/dh4096.pem"));
        settings()->set_ssl_settings(ssl_settings());
    }

private:
    std::map<std::string, std::string> _storage;

    void RestStoragePost(const std::shared_ptr<restbed::Session> session)
    {
        auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [this, request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "POST /storage/" << key << ": " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    void RestStorageGet(const std::shared_ptr<restbed::Session> session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "GET /storage/" << key << ": " << data << std::endl;

        session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
    }

    void RestStoragePut(const std::shared_ptr<restbed::Session> session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [this, request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /storage/" << key << ": " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    void RestStorageDelete(const std::shared_ptr<restbed::Session> session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "DELETE /storage/" << key << ": " << data << std::endl;

        _storage[key] = "";

        session->close(restbed::OK);
    }
};

int main(int argc, char** argv)
{
    // REST HTTPS server port
    int port = 8001;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "REST HTTPS server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new REST HTTPS server
    auto server = std::make_shared<RestServerSSL>(service, port);

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
