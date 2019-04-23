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

std::future<HTTPResponse> HTTPClientEx::MakeRequest(const HTTPRequest& request)
{
    // Create TCP resolver if the current one is empty
    if (!_resolver)
        _resolver = std::make_shared<Asio::TCPResolver>(service());

    _promise = std::promise<HTTPResponse>();
    _request = request;

    // Connect to Web server
    if (!ConnectAsync(_resolver))
        _promise.set_exception(std::make_exception_ptr(std::runtime_error("Connection failed!")));

    return _promise.get_future();
}

void HTTPClientEx::onConnected()
{
    if (!SendRequestAsync())
        _promise.set_exception(std::make_exception_ptr(std::runtime_error("Send HTTP request failed!")));
}

void HTTPClientEx::onReceivedResponse(const HTTPResponse& response)
{
    _promise.set_value(response);
}

void HTTPClientEx::onReceivedResponseError(const HTTPResponse& response, const std::string& error)
{
    _promise.set_exception(std::make_exception_ptr(std::runtime_error(error)));
}

} // namespace HTTP
} // namespace CppServer
