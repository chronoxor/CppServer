/*!
    \file https_session.h
    \brief HTTPS session definition
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTPS_SESSION_H
#define CPPSERVER_HTTP_HTTPS_SESSION_H

#include "http_request.h"
#include "http_response.h"

#include "cache/filecache.h"
#include "server/asio/ssl_session.h"

namespace CppServer {
namespace HTTP {

class HTTPSServer;

//! HTTPS session
/*!
    HTTPS session is used to receive/send HTTP requests/responses from the connected HTTPS client.

    Thread-safe.
*/
class HTTPSSession : public Asio::SSLSession
{
public:
    explicit HTTPSSession(const std::shared_ptr<HTTPSServer>& server);
    HTTPSSession(const HTTPSSession&) = delete;
    HTTPSSession(HTTPSSession&&) = delete;
    virtual ~HTTPSSession() = default;

    HTTPSSession& operator=(const HTTPSSession&) = delete;
    HTTPSSession& operator=(HTTPSSession&&) = delete;

    //! Get the static content cache
    CppCommon::FileCache& cache() noexcept { return _cache; }
    const CppCommon::FileCache& cache() const noexcept { return _cache; }

    //! Get the HTTP response
    HTTPResponse& response() noexcept { return _response; }
    const HTTPResponse& response() const noexcept { return _response; }

    //! Send the current HTTP response (synchronous)
    /*!
        \return Size of sent data
    */
    size_t SendResponse() { return SendResponse(_response); }
    //! Send the HTTP response (synchronous)
    /*!
        \param response - HTTP response
        \return Size of sent data
    */
    size_t SendResponse(const HTTPResponse& response) { return Send(response.cache()); }

    //! Send the HTTP response body (synchronous)
    /*!
        \param body - HTTP response body
        \return Size of sent data
    */
    size_t SendResponseBody(std::string_view body) { return Send(body); }
    //! Send the HTTP response body (synchronous)
    /*!
        \param buffer - HTTP response body buffer
        \param size - HTTP response body size
        \return Size of sent data
    */
    size_t SendResponseBody(const void* buffer, size_t size) { return Send(buffer, size); }

    //! Send the current HTTP response with timeout (synchronous)
    /*!
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendResponse(const CppCommon::Timespan& timeout) { return SendResponse(_response, timeout); }
    //! Send the HTTP response with timeout (synchronous)
    /*!
        \param response - HTTP response
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendResponse(const HTTPResponse& response, const CppCommon::Timespan& timeout) { return Send(response.cache(), timeout); }

    //! Send the HTTP response body with timeout (synchronous)
    /*!
        \param body - HTTP response body
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendResponseBody(std::string_view body, const CppCommon::Timespan& timeout) { return Send(body, timeout); }
    //! Send the HTTP response body with timeout (synchronous)
    /*!
        \param buffer - HTTP response body buffer
        \param size - HTTP response body size
        \param timeout - Timeout
        \return Size of sent data
    */
    size_t SendResponseBody(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { return Send(buffer, size, timeout); }

    //! Send the current HTTP response (asynchronous)
    /*!
        \return 'true' if the current HTTP response was successfully sent, 'false' if the session is not connected
    */
    bool SendResponseAsync() { return SendResponseAsync(_response); }
    //! Send the HTTP response (asynchronous)
    /*!
        \param response - HTTP response
        \return 'true' if the current HTTP response was successfully sent, 'false' if the session is not connected
    */
    bool SendResponseAsync(const HTTPResponse& response) { return SendAsync(response.cache()); }

    //! Send the HTTP response body (asynchronous)
    /*!
        \param body - HTTP response body
        \return 'true' if the current HTTP response was successfully sent, 'false' if the session is not connected
    */
    bool SendResponseBodyAsync(std::string_view body) { return SendAsync(body); }
    //! Send the HTTP response body (asynchronous)
    /*!
        \param buffer - HTTP response body buffer
        \param size - HTTP response body size
        \return 'true' if the current HTTP response was successfully sent, 'false' if the session is not connected
    */
    bool SendResponseBodyAsync(const void* buffer, size_t size) { return SendAsync(buffer, size); }

protected:
    void onReceived(const void* buffer, size_t size) override;
    void onDisconnected() override;

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
    //! Handle HTTP cached request received notification
    /*!
        Notification is called when HTTP request was received
        from the client and the corresponding cached content
        was found.

        Default behavior is just send cached response content
        to the client.

        \param request - HTTP request
        \param content - Cached response content
    */
    virtual void onReceivedCachedRequest(const HTTPRequest& request, std::string_view content) { SendAsync(content); }

    //! Handle HTTP request error notification
    /*!
        Notification is called when HTTP request error was received
        from the client.

        \param request - HTTP request
        \param error - HTTP request error
    */
    virtual void onReceivedRequestError(const HTTPRequest& request, const std::string& error) {}

protected:
    //! HTTP request
    HTTPRequest _request;
    //! HTTP response
    HTTPResponse _response;

private:
    // Static content cache
    CppCommon::FileCache& _cache;

    void onReceivedRequestInternal(const HTTPRequest& request);
};

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTPS_SESSION_H
