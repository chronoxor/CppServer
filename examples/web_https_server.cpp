/*!
    \file web_https_server.cpp
    \brief HTTPS Web server example
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/web_server.h"

#include <iostream>
#include <memory>
#include <map>

class HttpsServer : public CppServer::Asio::WebServer
{
public:
    explicit HttpsServer(std::shared_ptr<CppServer::Asio::Service> service, int port)
        : CppServer::Asio::WebServer(service, port, true)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage/{key: .*}");
        resource->set_method_handler("POST", RestStoragePost);
        resource->set_method_handler("GET", RestStorageGet);
        resource->set_method_handler("PUT", RestStoragePut);
        resource->set_method_handler("DELETE", RestStorageDelete);

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
    }

private:
    static std::map<std::string, std::string> _storage;

    static void RestStoragePost(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "POST /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageGet(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "GET /storage/" << key << " => " << data << std::endl;

        session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
    }

    static void RestStoragePut(const std::shared_ptr<restbed::Session>& session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageDelete(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "DELETE /storage/" << key << " => " << data << std::endl;

        _storage[key] = "";

        session->close(restbed::OK);
    }
};

std::map<std::string, std::string> HttpsServer::_storage;

int main(int argc, char** argv)
{
    // HTTPS Web server port
    int port = 9000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "HTTPS Web server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTPS Web server
    auto server = std::make_shared<HttpsServer>(service, port);

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
