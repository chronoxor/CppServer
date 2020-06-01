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

#include <sstream>
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

    //! Is the HTTP request empty?
    bool empty() const noexcept { return _cache.empty(); }
    //! Is the HTTP request error flag set?
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
    //! Get the HTTP request cookies count
    size_t cookies() const noexcept { return _cookies.size(); }
    //! Get the HTTP request cookie by index
    std::tuple<std::string_view, std::string_view> cookie(size_t i) const noexcept;
    //! Get the HTTP request body
    std::string_view body() const noexcept { return std::string_view(_cache.data() + _body_index, _body_size); }
    //! Get the HTTP request body length
    size_t body_length() const noexcept { return _body_length; }

    //! Get the HTTP request cache content
    const std::string& cache() const noexcept { return _cache; }

    //! Get string from the current HTTP request
    std::string string() const { std::stringstream ss; ss << *this; return ss.str(); }

    //! Clear the HTTP request cache
    HTTPRequest& Clear();

    //! Set the HTTP request begin with a given method, URL and protocol
    /*!
        \param method - HTTP method
        \param url - Requested URL
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    HTTPRequest& SetBegin(std::string_view method, std::string_view url, std::string_view protocol = "HTTP/1.1");
    //! Set the HTTP request header
    /*!
        \param key - Header key
        \param value - Header value
    */
    HTTPRequest& SetHeader(std::string_view key, std::string_view value);
    //! Set the HTTP request cookie
    /*!
        \param name - Cookie name
        \param value - Cookie value
    */
    HTTPRequest& SetCookie(std::string_view name, std::string_view value);
    //! Add the HTTP request cookie
    /*!
        \param name - Cookie name
        \param value - Cookie value
    */
    HTTPRequest& AddCookie(std::string_view name, std::string_view value);
    //! Set the HTTP request body
    /*!
        \param body - Body content (default is "")
    */
    HTTPRequest& SetBody(std::string_view body = "");
    //! Set the HTTP request body length
    /*!
        \param length - Body length
    */
    HTTPRequest& SetBodyLength(size_t length);

    //! Make HEAD request
    /*!
        \param url - URL to request
        \return HTTP request
    */
    HTTPRequest& MakeHeadRequest(std::string_view url);
    //! Make GET request
    /*!
        \param url - URL to request
        \return HTTP request
    */
    HTTPRequest& MakeGetRequest(std::string_view url);
    //! Make POST request
    /*!
        \param url - URL to request
        \param content - Content
        \param content_type - Content type (default is "text/plain; charset=UTF-8")
        \return HTTP request
    */
    HTTPRequest& MakePostRequest(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");
    //! Make PUT request
    /*!
        \param url - URL to request
        \param content - Content
        \param content_type - Content type (default is "text/plain; charset=UTF-8")
        \return HTTP request
    */
    HTTPRequest& MakePutRequest(std::string_view url, std::string_view content, std::string_view content_type = "text/plain; charset=UTF-8");
    //! Make DELETE request
    /*!
        \param url - URL to request
        \return HTTP request
    */
    HTTPRequest& MakeDeleteRequest(std::string_view url);
    //! Make OPTIONS request
    /*!
        \param url - URL to request
        \return HTTP request
    */
    HTTPRequest& MakeOptionsRequest(std::string_view url);
    //! Make TRACE request
    /*!
        \param url - URL to request
        \return HTTP request
    */
    HTTPRequest& MakeTraceRequest(std::string_view url);

    //! Output instance into the given output stream
    friend std::ostream& operator<<(std::ostream& os, const HTTPRequest& request);

    //! Swap two instances
    void swap(HTTPRequest& request) noexcept;
    friend void swap(HTTPRequest& request1, HTTPRequest& request2) noexcept { request1.swap(request2); }

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
    // HTTP request cookies
    std::vector<std::tuple<size_t, size_t, size_t, size_t>> _cookies;
    // HTTP request body
    size_t _body_index;
    size_t _body_size;
    size_t _body_length;
    bool _body_length_provided;

    // HTTP request cache
    std::string _cache;
    size_t _cache_size;

    // Is pending parts of HTTP response
    bool IsPendingHeader() const;
    bool IsPendingBody() const;

    // Receive parts of HTTP response
    bool ReceiveHeader(const void* buffer, size_t size);
    bool ReceiveBody(const void* buffer, size_t size);

    // Fast convert integer value to the corresponding string representation
    std::string_view FastConvert(size_t value, char* buffer, size_t size);
};

} // namespace HTTP
} // namespace CppServer

#include "http_request.inl"

#endif // CPPSERVER_HTTP_HTTP_REQUEST_H
