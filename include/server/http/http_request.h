/*!
    \file http_request.h
    \brief HTTP request definition
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_REQUEST_H
#define CPPSERVER_HTTP_HTTP_REQUEST_H

#include <map>
#include <string>
#include <string_view>

namespace CppServer {
namespace Http {

//! HTTP request
/*!
    HTTP request is used to create or process parameters
    of HTTP protocol request (method, URL, headers, etc).

    Not thread-safe.
*/
class HttpRequest
{
public:
    //! Initialize an empty HTTP request
    HttpRequest() = default;
    //! Initialize a new HTTP request with a given method, URL and protocol
    /*!
        \param method - HTTP method
        \param url - Requested URL
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    HttpRequest(const std::string_view& method, const std::string_view& url, const std::string_view& protocol = "HTTP/1.1") { Set(method, url, protocol); }
    HttpRequest(const HttpRequest&) = default;
    HttpRequest(HttpRequest&&) = default;
    ~HttpRequest() = default;

    HttpRequest& operator=(const HttpRequest&) = default;
    HttpRequest& operator=(HttpRequest&&) = default;

    //! Get the HTTP method
    const std::string_view& method() const noexcept { return _method; }
    //! Get the requested URL
    const std::string_view& url() const noexcept { return _url; }
    //! Get the protocol version
    const std::string_view& protocol() const noexcept { return _protocol; }
    //! Get HTTP headers
    const std::multimap<std::string_view, std::string_view>& headers() const noexcept { return _headers; }

    //! Get HTTP request cache
    const std::string& cache() const noexcept { return _cache; }

    //! Clear the HTTP request
    void Clear();

    //! Set the HTTP request with a given method, URL and protocol
    /*!
        \param method - HTTP method
        \param url - Requested URL
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    void Set(const std::string_view& method, const std::string_view& url, const std::string_view& protocol = "HTTP/1.1");
    //! Set the HTTP request header
    /*!
        \param key - Header key
        \param value - Header value
    */
    void SetHeader(const std::string_view& key, const std::string_view& value);

private:
    // HTTP request cache
    std::string _cache;
    // HTTP request method
    std::string_view _method;
    // HTTP request URL
    std::string_view _url;
    // HTTP request protocol
    std::string_view _protocol;
    // HTTP request headers
    std::multimap<std::string_view, std::string_view> _headers;
};

} // namespace Http
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_REQUEST_H
