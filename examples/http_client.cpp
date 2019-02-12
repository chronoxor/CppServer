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
    auto client = std::make_shared<CppServer::HTTP::HTTPClient>(service, address, "http");

    // Prepare HTTP request
    client->request().SetBegin("GET", "/");
    client->request().SetHeader("Host", "example.com");
    client->request().SetHeader("User-Agent", "Mozilla/5.0");
    client->request().SetBody();

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect(std::make_shared<CppServer::Asio::TCPResolver>(service));
    std::cout << "Done!" << std::endl;

    // Send HTTP request
    std::cout << "Send HTTP request...";
    client->SendRequest();
    std::cout << "Done!" << std::endl;

    // Receive HTTP response
    std::cout << "Receive HTTP response...";
    std::string response;
    std::string part;
    do
    {
        part = client->Receive(4096, CppCommon::Timespan::seconds(1));
        response += part;
    } while (!part.empty());
    std::cout << "Done!" << std::endl;

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    // Show HTTP response content
    std::cout << response;

    return 0;
}
