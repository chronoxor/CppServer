/*!
    \file http_response.h
    \brief HTTP response definition
    \author Ivan Shynkarenka
    \date 15.02.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_RESPONSE_H
#define CPPSERVER_HTTP_HTTP_RESPONSE_H

#include "http.h"

#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace CppServer {
namespace HTTP {

//! HTTP response
/*!
    HTTP response is used to create or process parameters
    of HTTP protocol response (status, headers, etc).

    Not thread-safe.
*/
class HTTPResponse
{
public:
    //! Initialize an empty HTTP response
    HTTPResponse() = default;
    //! Initialize a new HTTP response with a given status and protocol
    /*!
        \param status - HTTP status
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    HTTPResponse(int status, std::string_view protocol = "HTTP/1.1") { SetBegin(status, protocol); }
    //! Initialize a new HTTP response with a given status, status phrase and protocol
    /*!
        \param status - HTTP status
        \param status_phrase - HTTP status phrase
        \param protocol - Protocol version
    */
    HTTPResponse(int status, std::string_view status_phrase, std::string_view protocol) { SetBegin(status, status_phrase, protocol); }
    HTTPResponse(const HTTPResponse&) = default;
    HTTPResponse(HTTPResponse&&) = default;
    ~HTTPResponse() = default;

    HTTPResponse& operator=(const HTTPResponse&) = default;
    HTTPResponse& operator=(HTTPResponse&&) = default;

    //! Get the HTTP response status
    int status() const noexcept { return _status; }
    //! Get the HTTP response status phrase
    std::string_view status_phrase() const noexcept { return std::string_view(_cache.data() + _status_phrase_index, _status_phrase_size); }
    //! Get the HTTP response protocol version
    std::string_view protocol() const noexcept { return std::string_view(_cache.data() + _protocol_index, _protocol_size); }
    //! Get the HTTP response headers count
    size_t headers() const noexcept { return _headers.size(); }
    //! Get the HTTP response header by index
    std::tuple<std::string_view, std::string_view> header(size_t i) const noexcept;
    //! Get the HTTP response body
    std::string_view body() const noexcept { return std::string_view(_cache.data() + _body_index, _body_size); }
    //! Get the HTTP response body length
    size_t body_length() const noexcept { return _body_length; }

    //! Get the HTTP response cache content
    const std::string& cache() const noexcept { return _cache; }

    //! Clear the HTTP response cache
    void Clear();

    //! Set the HTTP response begin with a given status and protocol
    /*!
        \param status - HTTP status
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    void SetBegin(int status, std::string_view protocol = "HTTP/1.1");
    //! Set the HTTP response begin with a given status, status phrase and protocol
    /*!
        \param status - HTTP status
        \param status_phrase - HTTP status phrase
        \param protocol - Protocol version
    */
    void SetBegin(int status, std::string_view status_phrase, std::string_view protocol);
    //! Set the HTTP response header
    /*!
        \param key - Header key
        \param value - Header value
    */
    void SetHeader(std::string_view key, std::string_view value);
    //! Set the HTTP response body
    /*!
        \param body - Body content (default is "")
    */
    void SetBody(std::string_view body = "");
    //! Set the HTTP response body length
    /*!
        \param length - Body length
    */
    void SetBodyLength(size_t length);

private:
    // HTTP response status
    int _status;
    // HTTP response status phrase
    size_t _status_phrase_index;
    size_t _status_phrase_size;
    // HTTP response protocol
    size_t _protocol_index;
    size_t _protocol_size;
    // HTTP response headers
    std::vector<std::tuple<size_t, size_t, size_t, size_t>> _headers;
    // HTTP response body
    size_t _body_index;
    size_t _body_size;
    size_t _body_length;

    // HTTP response cache
    std::string _cache;
};

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_RESPONSE_H
