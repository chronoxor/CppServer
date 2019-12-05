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

bool WSClient::Connect(const std::shared_ptr<Asio::TCPResolver>& resolver)
{
    _sync_connect = true;
    return HTTPClient::Connect(resolver);
}

bool WSClient::ConnectAsync()
{
    _sync_connect = false;
    return HTTPClient::ConnectAsync();
}

bool WSClient::ConnectAsync(const std::shared_ptr<Asio::TCPResolver>& resolver)
{
    _sync_connect = false;
    return HTTPClient::ConnectAsync(resolver);
}

void WSClient::onConnected()
{
    // Clear WebSocket send/receive buffers
    ClearWSBuffers();

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

void WSClient::onReceived(const void* buffer, size_t size)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame
        PrepareReceiveFrame(buffer, size);
        return;
    }

    HTTPClient::onReceived(buffer, size);
}

void WSClient::onReceivedResponseHeader(const HTTP::HTTPResponse& response)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformClientUpgrade(response, id()))
    {
        HTTPClient::onReceivedResponseHeader(response);
        return;
    }
}

void WSClient::onReceivedResponse(const HTTP::HTTPResponse& response)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame from the remaining request body
        auto body = _request.body();
        PrepareReceiveFrame(body.data(), body.size());
        return;
    }

    HTTPClient::onReceivedResponse(response);
}

void WSClient::onReceivedResponseError(const HTTP::HTTPResponse& response, const std::string& error)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        onError(asio::error::fault, "WebSocket error", error);
        return;
    }

    HTTPClient::onReceivedResponseError(response, error);
}

std::string WSClient::ReceiveText()
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
        size_t received = HTTPClient::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::string WSClient::ReceiveText(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPClient::Receive(cache.data(), required, timeout);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSClient::ReceiveBinary()
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
        size_t received = HTTPClient::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSClient::ReceiveBinary(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPClient::Receive(cache.data(), required, timeout);
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
