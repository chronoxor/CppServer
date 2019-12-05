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

bool WSSClient::Connect(const std::shared_ptr<Asio::TCPResolver>& resolver)
{
    _sync_connect = true;
    return HTTPSClient::Connect(resolver);
}

bool WSSClient::ConnectAsync()
{
    _sync_connect = false;
    return HTTPSClient::ConnectAsync();
}

bool WSSClient::ConnectAsync(const std::shared_ptr<Asio::TCPResolver>& resolver)
{
    _sync_connect = false;
    return HTTPSClient::ConnectAsync(resolver);
}

void WSSClient::onHandshaked()
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

void WSSClient::onDisconnected()
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

void WSSClient::onReceived(const void* buffer, size_t size)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame
        PrepareReceiveFrame(buffer, size);
        return;
    }

    HTTPSClient::onReceived(buffer, size);
}

void WSSClient::onReceivedResponseHeader(const HTTP::HTTPResponse& response)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
        return;

    // Try to perform WebSocket upgrade
    if (!PerformClientUpgrade(response, id()))
    {
        HTTPSClient::onReceivedResponseHeader(response);
        return;
    }
}

void WSSClient::onReceivedResponse(const HTTP::HTTPResponse& response)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        // Prepare receive frame from the remaining request body
        auto body = _request.body();
        PrepareReceiveFrame(body.data(), body.size());
        return;
    }

    HTTPSClient::onReceivedResponse(response);
}

void WSSClient::onReceivedResponseError(const HTTP::HTTPResponse& response, const std::string& error)
{
    // Check for WebSocket handshaked status
    if (_ws_handshaked)
    {
        onError(asio::error::fault, "WebSocket error", error);
        return;
    }

    HTTPSClient::onReceivedResponseError(response, error);
}

std::string WSSClient::ReceiveText()
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
        size_t received = HTTPSClient::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::string WSSClient::ReceiveText(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPSClient::Receive(cache.data(), required, timeout);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSClient::ReceiveBinary()
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
        size_t received = HTTPSClient::Receive(cache.data(), required);
        if (received != required)
            return result;
        PrepareReceiveFrame(cache.data(), received);
    }

    // Copy WebSocket frame data
    result.insert(result.end(), _ws_receive_buffer.data() + _ws_header_size, _ws_receive_buffer.data() + _ws_header_size + _ws_payload_size);
    PrepareReceiveFrame(nullptr, 0);
    return result;
}

std::vector<uint8_t> WSSClient::ReceiveBinary(const CppCommon::Timespan& timeout)
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
        size_t received = HTTPSClient::Receive(cache.data(), required, timeout);
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
