/*!
    \file wss_chat_client.cpp
    \brief WebSocket secure chat client example
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/ws/wss_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::WS::WSSClient
{
public:
    using CppServer::WS::WSSClient::WSSClient;

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onWSConnecting(CppServer::HTTP::HTTPRequest& request) override
    {
        request.SetHeader("Host", "echo.websocket.org");
        request.SetHeader("Origin", "https://websocket.org");
    }

    void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override
    {
        std::cout << "Chat WebSocket secure client connected a new session with Id " << id() << std::endl;
    }

    void onWSDisconnected() override
    {
        std::cout << "Chat WebSocket secure client disconnected a session with Id " << id() << std::endl;
    }

    void onDisconnected() override
    {
        WSSClient::onDisconnected();

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket secure client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop{false};
};

int main(int argc, char** argv)
{
    // WebSocket server address
    std::string address = "174.129.224.73";
    if (argc > 1)
        address = argv[1];

    // WebSocket server port
    int port = 443;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "WebSocket secure server address: " << address << std::endl;
    std::cout << "WebSocket secure server port: " << port << std::endl;

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
    context->set_verify_callback(asio::ssl::rfc2818_verification("echo.websocket.org"));

    // Create a new WebSocket chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Reconnect the client
        if (line == "!")
        {
            std::cout << "Client reconnecting...";
            client->ReconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
