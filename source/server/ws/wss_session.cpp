/*!
    \file wss_session.cpp
    \brief WebSocket secure session implementation
    \author Ivan Shynkarenka
    \date 28.05.2019
    \copyright MIT License
*/

#include "server/ws/wss_session.h"
#include "server/ws/wss_server.h"

namespace CppServer {
namespace WS {

WSSSession::WSSSession(const std::shared_ptr<WSSServer>& server)
    : HTTP::HTTPSSession(server)
{
}

void WSSSession::onDisconnected()
{
    // Disconnect WebSocket
    if (_ws_handshaked)
    {
        _ws_handshaked = false;
        onWSDisconnected();
    }

    // Reset WebSocket upgrade HTTP request and response
    _request.Clear();
    _response.Clear();

    // Clear WebSocket send/receive buffers
    ClearWSBuffers();
}

void WSSSession::onReceived(const void* buffer, size_t size)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame
        PrepareReceiveFrame(buffer, size);
        return;
    }

    HTTPSSession::onReceived(buffer, size);
}

void WSSSession::onReceivedRequestHeader(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformServerUpgrade(request, response()))
    {
        HTTPSSession::onReceivedRequestHeader(request);
        return;
    }
}

void WSSSession::onReceivedRequest(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame from the remaining request body
        auto body = _request.body();
        PrepareReceiveFrame(body.data(), body.size());
        return;
    }

    HTTPSSession::onReceivedRequest(request);
}

void WSSSession::onReceivedRequestError(const HTTP::HTTPRequest& request, const std::string& error)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        onError(asio::error::fault, "WebSocket error", error);
        return;
    }

    HTTPSSession::onReceivedRequestError(request, error);
}

std::string WSSSession::ReceiveText()
{
    std::string result;

    if (!_ws_handshaked)
        return result;

    std::vector<uint8_t> cache;

    // Receive WebSocket frame data
    while (!_ws_received)
    {
        size_t required = RequiredReceiveFrameSize();
        cache.resize(required);
        size_t received = HTTPSSession::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::string WSSSession::ReceiveText(const CppCommon::Timespan& timeout)
{
    std::string result;

    if (!_ws_handshaked)
        return result;

    std::vector<uint8_t> cache;

    // Receive WebSocket frame data
    while (!_ws_received)
    {
        size_t required = RequiredReceiveFrameSize();
        cache.resize(required);
        size_t received = HTTPSSession::Receive(cache.data(), required, timeout);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSSession::ReceiveBinary()
{
    std::vector<uint8_t> result;

    if (!_ws_handshaked)
        return result;

    std::vector<uint8_t> cache;

    // Receive WebSocket frame data
    while (!_ws_received)
    {
        size_t required = RequiredReceiveFrameSize();
        cache.resize(required);
        size_t received = HTTPSSession::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSSession::ReceiveBinary(const CppCommon::Timespan& timeout)
{
    std::vector<uint8_t> result;

    if (!_ws_handshaked)
        return result;

    std::vector<uint8_t> cache;

    // Receive WebSocket frame data
    while (!_ws_received)
    {
        size_t required = RequiredReceiveFrameSize();
        cache.resize(required);
        size_t received = HTTPSSession::Receive(cache.data(), required, timeout);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

} // namespace WS
} // namespace CppServer
