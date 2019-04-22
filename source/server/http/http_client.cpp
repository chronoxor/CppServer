/*!
    \file http_client.cpp
    \brief HTTP client implementation
    \author Ivan Shynkarenka
    \date 08.02.2019
    \copyright MIT License
*/

#include "server/http/http_client.h"

namespace CppServer {
namespace HTTP {

void HTTPClient::onReceived(const void* buffer, size_t size)
{
    // Receive HTTP response header
    if (_response.IsPendingHeader())
    {
        if (_response.ReceiveHeader(buffer, size))
            onReceivedResponseHeader(_response);

        // Check for HTTP response error
        if (_response.error())
        {
            onReceivedResponseError(_response, "Invalid HTTP response!");
            DisconnectAsync();
            return;
        }

        return;
    }

    // Receive HTTP response body
    if (_response.ReceiveBody(buffer, size))
        onReceivedResponse(_response);

    // Check for HTTP response error
    if (_response.error())
    {
        onReceivedResponseError(_response, "Invalid HTTP response!");
        DisconnectAsync();
        return;
    }
}

void HTTPClient::onDisconnected()
{
    // Receive HTTP response body
    if (_response.IsPendingBody())
    {
        onReceivedResponse(_response);
        return;
    }
}

} // namespace HTTP
} // namespace CppServer
