/*!
    \file ws_client.cpp
    \brief WebSocket client implementation
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#include "server/ws/ws_client.h"

namespace CppServer {
namespace WS {

bool WSClient::Connect()
{
    _sync_connect = true;
    return HTTPClient::Connect();
}

bool WSClient::Connect(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = true;
    return HTTPClient::Connect(resolver);
}

bool WSClient::ConnectAsync()
{
    _sync_connect = false;
    return HTTPClient::ConnectAsync();
}

bool WSClient::ConnectAsync(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = false;
    return HTTPClient::ConnectAsync(resolver);
}

void WSClient::onConnected()
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

void WSClient::onDisconnected()
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

void WSClient::onReceived(const void* buffer, size_t size)
{
    // Perfrom the WebSocket handshake
    if (!_handshaked)
        HTTPClient::onReceived(buffer, size);
}

void WSClient::onReceivedResponseHeader(const HTTP::HTTPResponse& response)
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
