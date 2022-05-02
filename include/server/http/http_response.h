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

#include "time/time.h"

#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
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
    friend class HTTPClient;
    friend class HTTPSClient;

public:
    //! Initialize an empty HTTP response
    HTTPResponse() { Clear(); }
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

    //! Is the HTTP response empty?
    bool empty() const noexcept { return _cache.empty(); }
    //! Is the HTTP response error flag set?
    bool error() const noexcept { return _error; }

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

    //! Get string from the current HTTP response
    std::string string() const { std::stringstream ss; ss << *this; return ss.str(); }

    //! Clear the HTTP response cache
    HTTPResponse& Clear();

    //! Set the HTTP response begin with a given status and protocol
    /*!
        \param status - HTTP status
        \param protocol - Protocol version (default is "HTTP/1.1")
    */
    HTTPResponse& SetBegin(int status, std::string_view protocol = "HTTP/1.1");
    //! Set the HTTP response begin with a given status, status phrase and protocol
    /*!
        \param status - HTTP status
        \param status_phrase - HTTP status phrase
        \param protocol - Protocol version
    */
    HTTPResponse& SetBegin(int status, std::string_view status_phrase, std::string_view protocol);
    //! Set the HTTP response content type
    /*!
        \param extension - Content extension
    */
    HTTPResponse& SetContentType(std::string_view extension);
    //! Set the HTTP response header
    /*!
        \param key - Header key
        \param value - Header value
    */
    HTTPResponse& SetHeader(std::string_view key, std::string_view value);
    //! Set the HTTP response cookie
    /*!
        \param name - Cookie name
        \param value - Cookie value
        \param max_age - Cookie age in seconds until it expires (default is 86400)
        \param path - Cookie path (default is "")
        \param domain - Cookie domain (default is "")
        \param secure - Cookie secure flag (default is true)
        \param strict - Cookie strict flag (default is true)
        \param http_only - Cookie HTTP-only flag (default is true)
    */
    HTTPResponse& SetCookie(std::string_view name, std::string_view value, size_t max_age = 86400, std::string_view path = "", std::string_view domain = "", bool secure = true, bool strict = true, bool http_only = true);
    //! Set the HTTP response body
    /*!
        \param body - Body content (default is "")
    */
    HTTPResponse& SetBody(std::string_view body = "");
    //! Set the HTTP response body length
    /*!
        \param length - Body length
    */
    HTTPResponse& SetBodyLength(size_t length);

    //! Make OK response
    /*!
        \param status - OK status (default is 200 (OK))
        \return HTTP response
    */
    HTTPResponse& MakeOKResponse(int status = 200);
    //! Make ERROR response
    /*!
        \param content - Error content (default is "")
        \param content_type - Error content type (default is "text/plain; charset=UTF-8")
        \return HTTP response
    */
    HTTPResponse& MakeErrorResponse(std::string_view content = "", std::string_view content_type = "text/plain; charset=UTF-8") { return MakeErrorResponse(500, content, content_type); }
    //! Make ERROR response
    /*!
        \param status - Error status
        \param content - Error content (default is "")
        \param content_type - Error content type (default is "text/plain; charset=UTF-8")
        \return HTTP response
    */
    HTTPResponse& MakeErrorResponse(int status, std::string_view content = "", std::string_view content_type = "text/plain; charset=UTF-8");
    //! Make HEAD response
    /*!
        \return HTTP response
    */
    HTTPResponse& MakeHeadResponse();
    //! Make GET response
    /*!
        \param content - Content (default is "")
        \param content_type - Content type (default is "text/plain; charset=UTF-8")
        \return HTTP response
    */
    HTTPResponse& MakeGetResponse(std::string_view content = "", std::string_view content_type = "text/plain; charset=UTF-8");
    //! Make OPTIONS response
    /*!
        \param allow - Allow methods (default is "HEAD,GET,POST,PUT,DELETE,OPTIONS,TRACE")
        \return HTTP response
    */
    HTTPResponse& MakeOptionsResponse(std::string_view allow = "HEAD,GET,POST,PUT,DELETE,OPTIONS,TRACE");
    //! Make TRACE response
    /*!
        \param request - Request content
        \return HTTP response
    */
    HTTPResponse& MakeTraceResponse(std::string_view request);

    //! Output instance into the given output stream
    friend std::ostream& operator<<(std::ostream& os, const HTTPResponse& response);

    //! Swap two instances
    void swap(HTTPResponse& response) noexcept;
    friend void swap(HTTPResponse& response1, HTTPResponse& response2) noexcept { response1.swap(response2); }

private:
    // HTTP response error flag
    bool _error;
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
    bool _body_length_provided;

    // HTTP response cache
    std::string _cache;
    size_t _cache_size;

    // HTTP response mime table
    static const std::unordered_map<std::string, std::string> _mime_table;

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

#include "http_response.inl"

#endif // CPPSERVER_HTTP_HTTP_RESPONSE_H
