//
// Created by Ivan Shynkarenka on 08.02.2019
//

#include "test.h"

#include "server/http/http_client.h"
#include "server/http/https_client.h"
#include "threads/thread.h"

using namespace CppCommon;
using namespace CppServer::Asio;
using namespace CppServer::HTTP;

TEST_CASE("HTTP request test", "[CppServer][HTTP]")
{
    // Create a new HTTP request
    HTTPRequest request;

    // Prepare HTTP request
    request.SetBegin("PUT", "/");
    request.SetHeader("Host", "example.com");
    request.SetHeader("User-Agent", "Mozilla/5.0");
    request.SetBody("test");

    // Check the HTTP request state
    REQUIRE(request.method() == "PUT");
    REQUIRE(request.url() == "/");
    REQUIRE(request.protocol() == "HTTP/1.1");
    REQUIRE(request.headers() == 3);
    REQUIRE(std::get<0>(request.header(0)) == "Host");
    REQUIRE(std::get<1>(request.header(0)) == "example.com");
    REQUIRE(std::get<0>(request.header(1)) == "User-Agent");
    REQUIRE(std::get<1>(request.header(1)) == "Mozilla/5.0");
    REQUIRE(std::get<0>(request.header(2)) == "Content-Length");
    REQUIRE(std::get<1>(request.header(2)) == "4");
    REQUIRE(request.body() == "test");
}

TEST_CASE("HTTP response test", "[CppServer][HTTP]")
{
    // Create a new HTTP response
    HTTPResponse response;

    // Prepare HTTP response
    response.SetBegin(200);
    response.SetHeader("Accept-Ranges", "bytes");
    response.SetHeader("Content-Type", "text/html; charset=UTF-8");
    response.SetBody("test");

    // Check the HTTP response state
    REQUIRE(response.status() == 200);
    REQUIRE(response.status_phrase() == "OK");
    REQUIRE(response.protocol() == "HTTP/1.1");
    REQUIRE(response.headers() == 3);
    REQUIRE(std::get<0>(response.header(0)) == "Accept-Ranges");
    REQUIRE(std::get<1>(response.header(0)) == "bytes");
    REQUIRE(std::get<0>(response.header(1)) == "Content-Type");
    REQUIRE(std::get<1>(response.header(1)) == "text/html; charset=UTF-8");
    REQUIRE(std::get<0>(response.header(2)) == "Content-Length");
    REQUIRE(std::get<1>(response.header(2)) == "4");
    REQUIRE(response.body() == "test");
}

TEST_CASE("HTTP client test", "[CppServer][HTTP]")
{
    const std::string address = "example.com";

    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create a new HTTP client
    auto client = std::make_shared<HTTPClientEx>(service, address, "http");

    // Prepare HTTP request
    client->request().SetBegin("GET", "/");
    client->request().SetHeader("Host", "example.com");
    client->request().SetHeader("User-Agent", "Mozilla/5.0");
    client->request().SetBody();

    // Send HTTP request
    auto response = client->SendRequest().get();

    // Check HTTP response
    REQUIRE(response.status() == 200);
    REQUIRE(response.status_phrase() == "OK");
    REQUIRE(response.protocol() == "HTTP/1.1");
    REQUIRE(response.headers() > 0);
    REQUIRE(response.body_length() > 0);
    REQUIRE(response.body().size() > 0);

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();
}

TEST_CASE("HTTPS client test", "[CppServer][HTTP]")
{
    const std::string address = "example.com";

    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->set_verify_callback(asio::ssl::rfc2818_verification(address));

    // Create a new HTTP client
    auto client = std::make_shared<HTTPSClientEx>(service, context, address, "https");

    // Prepare HTTP request
    client->request().SetBegin("GET", "/");
    client->request().SetHeader("Host", "example.com");
    client->request().SetHeader("User-Agent", "Mozilla/5.0");
    client->request().SetBody();

    // Send HTTP request
    auto response = client->SendRequest().get();

    // Check HTTP response
    REQUIRE(response.status() == 200);
    REQUIRE(response.status_phrase() == "OK");
    REQUIRE(response.protocol() == "HTTP/1.1");
    REQUIRE(response.headers() > 0);
    REQUIRE(response.body_length() > 0);
    REQUIRE(response.body().size() > 0);

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();
}
