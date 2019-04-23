/*!
    \file https_client.cpp
    \brief HTTPS client implementation
    \author Ivan Shynkarenka
    \date 12.02.2019
    \copyright MIT License
*/

#include "server/http/https_client.h"

namespace CppServer {
namespace HTTP {

void HTTPSClient::onReceived(const void* buffer, size_t size)
{
    // Receive HTTP response header
    if (_response.IsPendingHeader())
    {
        if (_response.ReceiveHeader(buffer, size))
            onReceivedResponseHeader(_response);

        size = 0;
    }

    // Check for HTTP response error
    if (_response.error())
    {
        onReceivedResponseError(_response, "Invalid HTTP response!");
        _response.Clear();
        DisconnectAsync();
        return;
    }

    // Receive HTTP response body
    if (_response.ReceiveBody(buffer, size))
    {
        onReceivedResponse(_response);
        _response.Clear();
        return;
    }

    // Check for HTTP response error
    if (_response.error())
    {
        onReceivedResponseError(_response, "Invalid HTTP response!");
        _response.Clear();
        DisconnectAsync();
        return;
    }
}

void HTTPSClient::onDisconnected()
{
    // Receive HTTP response body
    if (_response.IsPendingBody())
    {
        onReceivedResponse(_response);
        _response.Clear();
        return;
    }
}

std::future<HTTPResponse> HTTPSClientEx::MakeRequest(const HTTPRequest& request)
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

void HTTPSClientEx::onHandshaked()
{
    if (!SendRequestAsync())
        _promise.set_exception(std::make_exception_ptr(std::runtime_error("Send HTTP request failed!")));
}

void HTTPSClientEx::onReceivedResponse(const HTTPResponse& response)
{
    _promise.set_value(response);
}

void HTTPSClientEx::onReceivedResponseError(const HTTPResponse& response, const std::string& error)
{
    _promise.set_exception(std::make_exception_ptr(std::runtime_error(error)));
}

} // namespace HTTP
} // namespace CppServer
