/*!
    \file http_client.h
    \brief HTTP client definition
    \author Ivan Shynkarenka
    \date 08.02.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_CLIENT_H
#define CPPSERVER_HTTP_HTTP_CLIENT_H

#include "http_request.h"

#include "server/asio/tcp_client.h"

namespace CppServer {
namespace HTTP {

//! HTTP client
/*!
    HTTP client is used to communicate with HTTP Web server.
    It allows to send GET, POST, PUT, DELETE requests and
    receive HTTP result.

    Thread-safe.
*/
class HTTPClient : public Asio::TCPClient
{
public:
    using TCPClient::TCPClient;

    HTTPClient(const HTTPClient&) = delete;
    HTTPClient(HTTPClient&&) = default;
    virtual ~HTTPClient() = default;

    HTTPClient& operator=(const HTTPClient&) = delete;
    HTTPClient& operator=(HTTPClient&&) = default;

    //! Get the HTTP request
    HTTPRequest& request() noexcept { return _request; }
    const HTTPRequest& request() const noexcept { return _request; }

    //! Send the current HTTP request (synchronous)
    /*!
        \return Size of sent data
    */
    size_t SendRequest() { return SendRequest(_request); }
    //! Send the HTTP request (synchronous)
    /*!
        \param request - HTTP request
        \return Size of sent data
    */
    size_t SendRequest(const HTTPRequest& request) { return Send(request.cache()); }

    //! Send the current HTTP request (asynchronous)
    /*!
        \return 'true' if the current HTTP request was successfully sent, 'false' if the client is not connected
    */
    bool SendRequestAsync() { return SendRequestAsync(_request); }
    //! Send the HTTP request (asynchronous)
    /*!
        \param request - HTTP request
        \return 'true' if the current HTTP request was successfully sent, 'false' if the client is not connected
    */
    bool SendRequestAsync(const HTTPRequest& request) { return SendAsync(request.cache()); }

private:
    // HTTP request
    HTTPRequest _request;
};

/*! \example http_client.cpp HTTP client example */

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_CLIENT_H
