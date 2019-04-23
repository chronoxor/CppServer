/*!
    \file http_client.cpp
    \brief HTTP client example
    \author Ivan Shynkarenka
    \date 08.02.2019
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/http/http_client.h"

#include <iostream>

int main(int argc, char** argv)
{
    // HTTP server address
    std::string address = "example.com";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTP server address: " << address << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP client
    auto client = std::make_shared<CppServer::HTTP::HTTPClientEx>(service, address, "http");

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
