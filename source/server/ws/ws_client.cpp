/*!
    \file ws_client.cpp
    \brief WebSocket client implementation
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#include "server/ws/ws_client.h"

#include "system/uuid.h"

namespace CppServer {
namespace WS {

bool WSClient::Connect()
{
    _sync_connect = true;
    return TCPClient::Connect();
}

bool WSClient::Connect(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = true;
    return TCPClient::Connect(resolver);
}

bool WSClient::ConnectAsync()
{
    _sync_connect = false;
    return TCPClient::ConnectAsync();
}

bool WSClient::ConnectAsync(std::shared_ptr<Asio::TCPResolver> resolver)
{
    _sync_connect = false;
    return TCPClient::ConnectAsync(resolver);
}

void WSClient::onConnected()
{
    // Fill WebSocket upgrade HTTP request
    _request.Clear();
    _request.SetBegin("GET", "/");
    _request.SetHeader("Upgrade", "websocket");
    _request.SetHeader("Connection", "Upgrade");
    _request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(id().string()));
    _request.SetHeader("Sec-WebSocket-Version", "13");

    // Allows to update WebSocket upgrade HTTP request in user code
    onWSConnecting(_request);

    // Set empty body of the WebSocket upgrade HTTP request
    _request.SetBody();

    // Send the WebSocket upgrade HTTP request
    if (_sync_connect)
        Send(_request.cache());
    else
        SendAsync(_request.cache());
}

} // namespace WS
} // namespace CppServer
