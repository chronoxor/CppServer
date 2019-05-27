/*!
    \file ws.cpp
    \brief WebSocket C++ Library implementation
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#include "server/ws/ws.h"

#include "string/encoding.h"
#include "string/format.h"

#include <algorithm>
#include <openssl/sha.h>

namespace CppServer {
namespace WS {

bool WebSocket::PerformUpgrade(const HTTP::HTTPResponse& response, const CppCommon::UUID& id)
{
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
                    onWSError("Invalid WebSocket handshaked response: 'Connection' header value must be 'Upgrade'");
                    break;
                }

                connection = true;
            }
            else if (key == "Upgrade")
            {
                if (value != "websocket")
                {
                    error = true;
                    onWSError("Invalid WebSocket handshaked response: 'Upgrade' header value must be 'websocket'");
                    break;
                }

                upgrade = true;
            }
            else if (key == "Sec-WebSocket-Accept")
            {
                // Calculate the original WebSocket hash
                std::string wskey = CppCommon::Encoding::Base64Encode(id.string()) + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                char wshash[SHA_DIGEST_LENGTH];
                SHA1((const unsigned char*)wskey.data(), wskey.size(), (unsigned char*)wshash);

                // Get the received WebSocket hash
                wskey = CppCommon::Encoding::Base64Decode(value);

                // Compare original and received hashes
                if (std::strncmp(wskey.data(), wshash, std::max(wskey.size(), sizeof(wshash))) != 0)
                {
                    error = true;
                    onWSError("Invalid WebSocket handshaked response: 'Sec-WebSocket-Accept' value validation failed");
                    break;
                }

                accept = true;
            }
        }

        // Failed to perfrom WebSocket handshake
        if (!accept || !connection || !upgrade)
        {
            if (!error)
                onWSError("Invalid WebSocket response");
            return false;
        }

        // WebSocket successfully handshaked!
        _handshaked = true;
        *((uint32_t*)_mask) = rand();
        onWSConnected(response);

        return true;
    }

    onWSError("Invalid WebSocket response status: {}"_format(response.status()));
    return false;
}

void WebSocket::PrepareSendFrame(uint8_t opcode, const void* buffer, size_t size, int status)
{
    // Clear the previous WebSocket send buffer
    _ws_send_buffer.clear();

    // Append WebSocket frame opcode
    _ws_send_buffer.push_back(opcode);

    // Append WebSocket frame size
    if (size <= 125)
        _ws_send_buffer.push_back((size & 0xFF) | 0x80);
    else if (size <= 65535)
    {
        _ws_send_buffer.push_back(126 | 0x80);
        _ws_send_buffer.push_back((size >> 8) & 0xFF);
        _ws_send_buffer.push_back(size & 0xFF);
    }
    else
    {
        _ws_send_buffer.push_back(127 | 0x80);
        for (int i = 3; i >= 0; --i)
            _ws_send_buffer.push_back(0);
        for (int i = 3; i >= 0; --i)
            _ws_send_buffer.push_back((size >> (8 * i)) & 0xFF);
    }

    // Append WebSocket frame mask
    _ws_send_buffer.push_back(_mask[0]);
    _ws_send_buffer.push_back(_mask[1]);
    _ws_send_buffer.push_back(_mask[2]);
    _ws_send_buffer.push_back(_mask[3]);

    // Resize WebSocket frame buffer
    size_t offset = _ws_send_buffer.size();
    _ws_send_buffer.resize(offset + size);

    // Pack WebSocket frame content
    const uint8_t* data = (const uint8_t*)buffer;
    for (size_t i = 0; i < size; ++i)
        _ws_send_buffer[offset + i] = data[i] ^ _mask[i % 4];
}

} // namespace WS
} // namespace CppServer
