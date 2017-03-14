//
// Created by Ivan Shynkarenka on 15.03.2017
//

#include "catch.hpp"

#include "server/asio/web_client.h"
#include "server/asio/web_server.h"
#include "threads/thread.h"

#include <memory>
#include <map>

using namespace CppCommon;
using namespace CppServer::Asio;

class HttpServer : public WebServer
{
public:
    explicit HttpServer(std::shared_ptr<Service> service, int port)
        : WebServer(service, port, false)
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

std::map<std::string, std::string> HttpServer::_storage;

TEST_CASE("HTTP Web server & client", "[CppServer][Asio]")
{
    const std::string address = "127.0.0.1";
    const int port = 8000;
    const std::string uri = "http://" + address + ":" + std::to_string(port) + "/storage/test";

    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start HTTP Web server
    auto server = std::make_shared<HttpServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create a new HTTP Web client
    auto client = std::make_shared<CppServer::Asio::WebClient>(service, false);

    // Create and fill Web request
    auto request = std::make_shared<restbed::Request>(restbed::Uri(address));
    request->set_header("Accept", "*/*");

    // Send a POST request to the HTTP Web server
    request->set_method("POST");
    request->set_body("123");
    auto response = client->Send(request);
    REQUIRE(response != nullptr);
    auto length = response->get_header("Content-Length", 0);
    WebClient::Fetch(response, length);
    REQUIRE(response->get_body().data().size() == 3);

    // Stop the HTTP Web server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();
}
