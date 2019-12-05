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

WSSession::WSSession(const std::shared_ptr<WSServer>& server)
    : HTTP::HTTPSession(server)
{
}

void WSSession::onDisconnected()
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

void WSSession::onReceived(const void* buffer, size_t size)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame
        PrepareReceiveFrame(buffer, size);
        return;
    }

    HTTPSession::onReceived(buffer, size);
}

void WSSession::onReceivedRequestHeader(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformServerUpgrade(request, response()))
    {
        HTTPSession::onReceivedRequestHeader(request);
        return;
    }
}

void WSSession::onReceivedRequest(const HTTP::HTTPRequest& request)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame from the remaining request body
        auto body = _request.body();
        PrepareReceiveFrame(body.data(), body.size());
        return;
    }

    HTTPSession::onReceivedRequest(request);
}

void WSSession::onReceivedRequestError(const HTTP::HTTPRequest& request, const std::string& error)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        onError(asio::error::fault, "WebSocket error", error);
        return;
    }

    HTTPSession::onReceivedRequestError(request, error);
}

std::string WSSession::ReceiveText()
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
        size_t received = HTTPSession::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::string WSSession::ReceiveText(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPSession::Receive(cache.data(), required, timeout);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSession::ReceiveBinary()
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
        size_t received = HTTPSession::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSession::ReceiveBinary(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPSession::Receive(cache.data(), required, timeout);
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
