/*!
    \file wss_client.cpp
    \brief WebSocket secure client implementation
    \author Ivan Shynkarenka
    \date 23.05.2019
    \copyright MIT License
*/

#include "server/ws/wss_client.h"

namespace CppServer {
namespace WS {

bool WSSClient::Connect()
{
    _sync_connect = true;
    return HTTPSClient::Connect();
}

bool WSSClient::Connect(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = true;
    return HTTPSClient::Connect(resolver);
}

bool WSSClient::ConnectAsync()
{
    _sync_connect = false;
    return HTTPSClient::ConnectAsync();
}

bool WSSClient::ConnectAsync(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = false;
    return HTTPSClient::ConnectAsync(resolver);
}

void WSSClient::onHandshaked()
{
    // Fill the WebSocket upgrade HTTP request
    onWSConnecting(_request);

    // Set empty body of the WebSocket upgrade HTTP request
    _request.SetBody();

    // Send the WebSocket upgrade HTTP request
    if (_sync_connect)
        Send(_request.cache());
    else
        SendAsync(_request.cache());
}

void WSSClient::onDisconnected()
{
    // Disconnect WebSocket
    if (_handshaked)
    {
        _handshaked = false;
        onWSDisconnected();
    }

    // Reset WebSocket upgrade HTTP request and response
    _request.Clear();
    _response.Clear();
}

void WSSClient::onReceived(const void* buffer, size_t size)
{
    // Perfrom the WebSocket handshake
    if (!_handshaked)
        HTTPSClient::onReceived(buffer, size);
}

void WSSClient::onReceivedResponseHeader(const HTTP::HTTPResponse& response)
{
    // Check for WebSocket handshaked status
    if (_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformUpgrade(response, id()))
    {
        DisconnectAsync();
        return;
    }
}

} // namespace WS
} // namespace CppServer
