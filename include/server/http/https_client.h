/*!
    \file https_client.h
    \brief HTTPS client definition
    \author Ivan Shynkarenka
    \date 12.02.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTPS_CLIENT_H
#define CPPSERVER_HTTP_HTTPS_CLIENT_H

#include "http_request.h"

#include "server/asio/ssl_client.h"

namespace CppServer {
namespace HTTP {

//! HTTPS client
/*!
    HTTPS client is used to communicate with HTTPS Web server.
    It allows to send GET, POST, PUT, DELETE requests and
    receive HTTP result using secure transport.

    Thread-safe.
*/
class HTTPSClient : public Asio::SSLClient
{
public:
    using SSLClient::SSLClient;

    HTTPSClient(const HTTPSClient&) = delete;
    HTTPSClient(HTTPSClient&&) = default;
    virtual ~HTTPSClient() = default;

    HTTPSClient& operator=(const HTTPSClient&) = delete;
    HTTPSClient& operator=(HTTPSClient&&) = default;

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

    //! Send the current HTTP request with timeout (synchronous)
    /*!
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendRequest(const CppCommon::Timespan& timeout) { return SendRequest(_request, timeout); }
    //! Send the HTTP request with timeout (synchronous)
    /*!
        \param request - HTTP request
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendRequest(const HTTPRequest& request, const CppCommon::Timespan& timeout) { return Send(request.cache(), timeout); }

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

/*! \example https_client.cpp HTTPS client example */

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTPS_CLIENT_H
