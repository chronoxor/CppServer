/*!
    \file web_http_client_sync.cpp
    \brief HTTP/HTTPS Web synchronous client example
    \author Ivan Shynkarenka
    \date 14.03.2017
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/web_client.h"

#include <iostream>
#include <memory>

void Show(const std::shared_ptr<restbed::Response>& response)
{
    std::cout << "*** Response header ***" << std::endl;
    std::cout << "Status Code:    " << response->get_status_code() << std::endl;
    std::cout << "Status Message: " << response->get_status_message().data() << std::endl;
    std::cout << "HTTP Version:   " << response->get_version() << std::endl;
    std::cout << "HTTP Protocol:  " << response->get_protocol().data() << std::endl;
    for (auto& header : response->get_headers())
        std::cout << "Header ['" << header.first.data() << "'] = '" << header.second.data() << "'" << std::endl;

    std::cout << "*** Response body ***" << std::endl;
    auto length = response->get_header("Content-Length", 0);
    auto content = CppServer::Asio::WebClient::Fetch(response, length);
    std::cout.write((char*)content.data(), content.size());
    std::cout << std::endl << "*** Response end ***" << std::endl;
}

int main(int argc, char** argv)
{
    // HTTP/HTTPS Web server address
    std::string address = "https://www.google.com";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTP/HTTPS Web server address: " << address << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP/HTTPS Web client
    auto client = std::make_shared<CppServer::Asio::WebClient>(service, false);

    // Create and fill Web request
    auto request = std::make_shared<restbed::Request>(restbed::Uri(address));
    request->set_header("Accept", "*/*");

    try
    {
        // Send synchronous Web request to the server
        auto response = client->Send(request);

        // Show the Web response
        Show(response);
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
