/*!
    \file http_session.h
    \brief HTTP session definition
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_SESSION_H
#define CPPSERVER_HTTP_HTTP_SESSION_H

#include "http_request.h"
#include "http_response.h"

#include "server/asio/tcp_session.h"

namespace CppServer {
namespace HTTP {

class HTTPServer;

//! HTTP session
/*!
    HTTP session is used to receive/send HTTP requests/responses from the connected HTTP client.

    Thread-safe.
*/
class HTTPSession : public Asio::TCPSession
{
public:
    using TCPSession::TCPSession;

    HTTPSession(const HTTPSession&) = delete;
    HTTPSession(HTTPSession&&) = delete;
    virtual ~HTTPSession() = default;

    HTTPSession& operator=(const HTTPSession&) = delete;
    HTTPSession& operator=(HTTPSession&&) = delete;

protected:
    //! Handle HTTP request header received notification
    /*!
        Notification is called when HTTP request header was received
        from the client.

        \param request - HTTP request
    */
    virtual void onReceivedRequestHeader(const HTTPRequest& request) {}

    //! Handle HTTP request received notification
    /*!
        Notification is called when HTTP request was received
        from the client.

        \param request - HTTP request
    */
    virtual void onReceivedRequest(const HTTPRequest& request) {}

    //! Handle HTTP request error notification
    /*!
        Notification is called when HTTP request error was received
        from the client.

        \param request - HTTP request
        \param error - HTTP request error
    */
    virtual void onReceivedRequestError(const HTTPRequest& request, const std::string& error) {}

private:
    // HTTP request
    HTTPRequest _request;
    // HTTP response
    HTTPResponse _response;
};

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_SESSION_H
