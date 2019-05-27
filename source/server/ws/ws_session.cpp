/*!
    \file ws_session.cpp
    \brief WebSocket session implementation
    \author Ivan Shynkarenka
    \date 27.05.2019
    \copyright MIT License
*/

#include "server/ws/ws_session.h"
#include "server/ws/ws_server.h"

namespace CppServer {
namespace WS {

WSSession::WSSession(std::shared_ptr<WSServer> server)
    : HTTP::HTTPSession(server)
{
}

void WSSession::onDisconnected()
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

    // Clear WebSocket send/receive buffers
    ClearWSBuffers();
}

void WSSession::onReceived(const void* buffer, size_t size)
{
    // Perfrom the WebSocket handshake
    if (!_handshaked)
    {
        HTTPSession::onReceived(buffer, size);
        return;
    }

    // Prepare receive frame
    PrepareReceiveFrame(buffer, size);
}

void WSSession::onReceivedRequestHeader(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformUpgrade(request))
    {
        Disconnect();
        return;
    }
}

void WSSession::onReceivedRequest(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_handshaked)
    {
        // Prepare receive frame from the remaining request body
        auto body = _request.body();
        PrepareReceiveFrame(body.data(), body.size());
    }
}

} // namespace WS
} // namespace CppServer
