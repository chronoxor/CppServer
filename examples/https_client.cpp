/*!
    \file https_client.cpp
    \brief HTTPS client example
    \author Ivan Shynkarenka
    \date 12.02.2019
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/http/https_client.h"

#include <iostream>

int main(int argc, char** argv)
{
    // HTTP server address
    std::string address = "example.com";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTPS server address: " << address << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->set_verify_callback(asio::ssl::rfc2818_verification(address));

    // Create a new HTTP client
    auto client = std::make_shared<CppServer::HTTP::HTTPSClientEx>(service, context, address, "https");

    // Prepare HTTP request
    client->request().SetBegin("GET", "/");
    client->request().SetHeader("Host", "example.com");
    client->request().SetHeader("User-Agent", "Mozilla/5.0");
    client->request().SetBody();

    // Send HTTP request
    std::cout << "Send HTTP request...";
    auto response = client->MakeRequest().get();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    // Show HTTP response content
    std::cout << "Status: " << response.status() << std::endl;
    std::cout << "Status phrase: " << response.status_phrase() << std::endl;
    std::cout << "Protocol: " << response.protocol() << std::endl;
    std::cout << "Headers: " << response.headers() << std::endl;
    for (size_t i = 0; i < response.headers(); ++i)
    {
        auto header = response.header(i);
        std::cout << std::get<0>(header) << ": " << std::get<1>(header) << std::endl;
    }
    std::cout << "Body:" << response.body_length() << std::endl;
    std::cout << response.body() << std::endl;

    return 0;
}
