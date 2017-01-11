/*!
    \file websocket_chat_client.cpp
    \brief WebSocket chat client example
    \author Ivan Shynkarenka
    \date 11.01.2016
    \copyright MIT License
*/

#include "server/asio/websocket_client.h"

#include <iostream>

class ChatClient : public CppServer::Asio::WebSocketClient
{
public:
    using CppServer::Asio::WebSocketClient::WebSocketClient;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocket client connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Chat WebSocket client disconnected a session with Id " << id() << std::endl;

        // Try to wait for a while
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Asio::WebSocketMessage message) override
    {
        std::cout << "Incoming: " << message->get_payload() << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // WebSocket server port
    int port = 4444;
    if (argc > 2)
        port = std::atoi(argv[2]);

    // WebSocket server uri
    std::string uri = "ws://" + address + ":" + std::to_string(port);

    std::cout << "WebSocket server address: " << address << std::endl;
    std::cout << "WebSocket server port: " << port << std::endl;
    std::cout << "WebSocket server uri: " << uri << std::endl;
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    service->Start();

    // Create a new WebSocket chat client
    auto client = std::make_shared<ChatClient>(service, uri);

    // Connect the client
    client->Connect();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Send the entered text to the chat server
        client->Send(line.data(), line.size());
    }

    // Disconnect the client
    client->Disconnect();

    // Stop the service
    service->Stop();

    return 0;
}
