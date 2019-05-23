/*!
    \file ws_client.cpp
    \brief WebSocket client implementation
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#include "server/ws/ws_client.h"

#include "string/format.h"
#include "system/uuid.h"

#include <algorithm>
#include <openssl/sha.h>

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
    _request.SetBegin("GET", "/");
    _request.SetHeader("Upgrade", "websocket");
    _request.SetHeader("Connection", "Upgrade");
    _request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(id().string()));
    _request.SetHeader("Sec-WebSocket-Protocol", "chat, superchat");
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

    // Try to perform WebSocket handshake
    if (response.status() == 101)
    {
        bool error = false;
        bool accept = false;
        bool connection = false;
        bool upgrade = false;

        for (size_t i = 0; i < response.headers(); ++i)
        {
            auto header = response.header(i);
            auto key = std::get<0>(header);
            auto value = std::get<1>(header);

            if (key == "Connection")
            {
                if (value != "Upgrade")
                {
                    error = true;
                    onError(asio::error::fault, "WebSocket error", "Invalid WebSocket handshaked response: 'Connection' header value must be 'Upgrade'");
                    break;
                }

                connection = true;
            }
            else if (key == "Upgrade")
            {
                if (value != "websocket")
                {
                    error = true;
                    onError(asio::error::fault, "WebSocket error", "Invalid WebSocket handshaked response: 'Upgrade' header value must be 'websocket'");
                    break;
                }

                upgrade = true;
            }
            else if (key == "Sec-WebSocket-Accept")
            {
                // Calculate the original WebSocket hash
                std::string wskey = CppCommon::Encoding::Base64Encode(id().string()) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                char wshash[SHA_DIGEST_LENGTH];
                SHA1((const unsigned char*)wskey.data(), wskey.size(), (unsigned char*)wshash);

                // Get the received WebSocket hash
                wskey = CppCommon::Encoding::Base64Decode(value);

                // Compare original and received hashes
                if (std::strncmp(wskey.data(), wshash, std::max(wskey.size(), sizeof(wshash))) != 0)
                {
                    error = true;
                    onError(asio::error::fault, "WebSocket error", "Invalid WebSocket handshaked response: 'Sec-WebSocket-Accept' value validation failed");
                    break;
                }

                accept = true;
            }
        }

        // Failed to perfrom WebSocket handshake
        if (!accept || !connection || !upgrade)
        {
            if (!error)
                onError(asio::error::fault, "WebSocket error", "Invalid WebSocket response");
            DisconnectAsync();
            return;
        }

        // WebSocket successfully handshaked!
        _handshaked = true;
        _mask = rand();
        onWSConnected(response);

        return;
    }

    // Disconnect on WebSocket handshake
    onError(asio::error::fault, "WebSocket error", "Invalid WebSocket response status: {}"_format(response.status()));
    DisconnectAsync();
}

void WSClient::PrepareWebSocketFrame(uint32_t opcode, const void* buffer, size_t size)
{
    _ws_send_buffer.clear();
}

} // namespace WS
} // namespace CppServer
