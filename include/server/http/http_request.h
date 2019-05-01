/*!
    \file http_request.h
    \brief HTTP request definition
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_REQUEST_H
#define CPPSERVER_HTTP_HTTP_REQUEST_H

#include "http.h"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace CppServer {
namespace HTTP {

//! HTTP request
/*!
    HTTP request is used to create or process parameters
    of HTTP protocol request (method, URL, headers, etc).

    Not thread-safe.
*/
class HTTPRequest
{
    friend class HTTPSession;
    friend class HTTPSSession;

public:
    //! Initialize an empty HTTP request
    HTTPRequest() { Clear(); }
    //! Initialize a new HTTP request with a given method, URL and protocol
    /*!
        \param method - HTTP method
        \param url - Requested URL
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    HTTPRequest(std::string_view method, std::string_view url, std::string_view protocol = "HTTP/1.1") { SetBegin(method, url, protocol); }
    HTTPRequest(const HTTPRequest&) = default;
    HTTPRequest(HTTPRequest&&) = default;
    ~HTTPRequest() = default;

    HTTPRequest& operator=(const HTTPRequest&) = default;
    HTTPRequest& operator=(HTTPRequest&&) = default;

    //! Get the HTTP request error flag
    bool error() const noexcept { return _error; }
    //! Get the HTTP request method
    std::string_view method() const noexcept { return std::string_view(_cache.data() + _method_index, _method_size); }
    //! Get the HTTP request URL
    std::string_view url() const noexcept { return std::string_view(_cache.data() + _url_index, _url_size); }
    //! Get the HTTP request protocol version
    std::string_view protocol() const noexcept { return std::string_view(_cache.data() + _protocol_index, _protocol_size); }
    //! Get the HTTP request headers count
    size_t headers() const noexcept { return _headers.size(); }
    //! Get the HTTP request header by index
    std::tuple<std::string_view, std::string_view> header(size_t i) const noexcept;
    //! Get the HTTP request body
    std::string_view body() const noexcept { return std::string_view(_cache.data() + _body_index, _body_size); }
    //! Get the HTTP request body length
    size_t body_length() const noexcept { return _body_length; }

    //! Get the HTTP request cache content
    const std::string& cache() const noexcept { return _cache; }

    //! Clear the HTTP request cache
    void Clear();

    //! Set the HTTP request begin with a given method, URL and protocol
    /*!
        \param method - HTTP method
        \param url - Requested URL
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    void SetBegin(std::string_view method, std::string_view url, std::string_view protocol = "HTTP/1.1");
    //! Set the HTTP request header
    /*!
        \param key - Header key
        \param value - Header value
    */
    void SetHeader(std::string_view key, std::string_view value);
    //! Set the HTTP request body
    /*!
        \param body - Body content (default is "")
    */
    void SetBody(std::string_view body = "");
    //! Set the HTTP request body length
    /*!
        \param length - Body length
    */
    void SetBodyLength(size_t length);

private:
    // HTTP request error flag
    bool _error;
    // HTTP request method
    size_t _method_index;
    size_t _method_size;
    // HTTP request URL
    size_t _url_index;
    size_t _url_size;
    // HTTP request protocol
    size_t _protocol_index;
    size_t _protocol_size;
    // HTTP request headers
    std::vector<std::tuple<size_t, size_t, size_t, size_t>> _headers;
    // HTTP request body
    size_t _body_index;
    size_t _body_size;
    size_t _body_length;

    // HTTP request cache
    std::string _cache;
    size_t _cache_size;

    // Is pending parts of HTTP response
    bool IsPendingHeader() const;
    bool IsPendingBody() const;

    // Receive parts of HTTP response
    bool ReceiveHeader(const void* buffer, size_t size);
    bool ReceiveBody(const void* buffer, size_t size);
};

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_REQUEST_H
