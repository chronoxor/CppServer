/*!
    \file http_request.cpp
    \brief HTTP request implementation
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

#include "server/http/http_request.h"

#include "string/string_utils.h"
#include "utility/countof.h"

#include <cassert>

namespace CppServer {
namespace HTTP {

std::tuple<std::string_view, std::string_view> HTTPRequest::header(size_t i) const noexcept
{
    assert((i < _headers.size()) && "Index out of bounds!");
    if (i >= _headers.size())
        return std::make_tuple(std::string_view(), std::string_view());

    auto item = _headers[i];

    return std::make_tuple(std::string_view(_cache.data() + std::get<0>(item), std::get<1>(item)), std::string_view(_cache.data() + std::get<2>(item), std::get<3>(item)));
}

std::tuple<std::string_view, std::string_view> HTTPRequest::cookie(size_t i) const noexcept
{
    assert((i < _cookies.size()) && "Index out of bounds!");
    if (i >= _cookies.size())
        return std::make_tuple(std::string_view(), std::string_view());

    auto item = _cookies[i];

    return std::make_tuple(std::string_view(_cache.data() + std::get<0>(item), std::get<1>(item)), std::string_view(_cache.data() + std::get<2>(item), std::get<3>(item)));
}

HTTPRequest& HTTPRequest::Clear()
{
    _error = false;
    _method_index = 0;
    _method_size = 0;
    _url_index = 0;
    _url_size = 0;
    _protocol_index = 0;
    _protocol_size = 0;
    _headers.clear();
    _cookies.clear();
    _body_index = 0;
    _body_size = 0;
    _body_length = 0;
    _body_length_provided = false;

    _cache.clear();
    _cache_size = 0;
    return *this;
}

HTTPRequest& HTTPRequest::SetBegin(std::string_view method, std::string_view url, std::string_view protocol)
{
    // Clear the HTTP request cache
    Clear();

    size_t index = 0;

    // Append the HTTP request method
    _cache.append(method);
    _method_index = index;
    _method_size = method.size();

    _cache.append(" ");
    index = _cache.size();

    // Append the HTTP request URL
    _cache.append(url);
    _url_index = index;
    _url_size = url.size();

    _cache.append(" ");
    index = _cache.size();

    // Append the HTTP request protocol version
    _cache.append(protocol);
    _protocol_index = index;
    _protocol_size = protocol.size();

    _cache.append("\r\n");
    return *this;
}

HTTPRequest& HTTPRequest::SetHeader(std::string_view key, std::string_view value)
{
    size_t index = _cache.size();

    // Append the HTTP request header's key
    _cache.append(key);
    size_t key_index = index;
    size_t key_size = key.size();

    _cache.append(": ");
    index = _cache.size();

    // Append the HTTP request header's value
    _cache.append(value);
    size_t value_index = index;
    size_t value_size = value.size();

    _cache.append("\r\n");

    // Add the header to the corresponding collection
    _headers.emplace_back(key_index, key_size, value_index, value_size);
    return *this;
}

HTTPRequest& HTTPRequest::SetCookie(std::string_view name, std::string_view value)
{
    size_t index = _cache.size();

    // Append the HTTP request header's key
    _cache.append("Cookie");
    size_t key_index = index;
    size_t key_size = 6;

    _cache.append(": ");
    index = _cache.size();

    // Append the HTTP request header's value
    size_t value_index = index;

    // Append Cookie
    index = _cache.size();
    _cache.append(name);
    size_t name_index = index;
    size_t name_size = name.size();
    _cache.append("=");
    index = _cache.size();
    _cache.append(value);
    size_t cookie_index = index;
    size_t cookie_size = value.size();

    size_t value_size = _cache.size() - value_index;

    _cache.append("\r\n");

    // Add the header to the corresponding collection
    _headers.emplace_back(key_index, key_size, value_index, value_size);
    // Add the cookie to the corresponding collection
    _cookies.emplace_back(name_index, name_size, cookie_index, cookie_size);
    return *this;
}

HTTPRequest& HTTPRequest::AddCookie(std::string_view name, std::string_view value)
{
    // Append Cookie
    _cache.append("; ");
    size_t index = _cache.size();
    _cache.append(name);
    size_t name_index = index;
    size_t name_size = name.size();
    _cache.append("=");
    index = _cache.size();
    _cache.append(value);
    size_t cookie_index = index;
    size_t cookie_size = value.size();

    // Add the cookie to the corresponding collection
    _cookies.emplace_back(name_index, name_size, cookie_index, cookie_size);
    return *this;
}

HTTPRequest& HTTPRequest::SetBody(std::string_view body)
{
    // Append content length header
    char buffer[32];
    SetHeader("Content-Length", FastConvert(body.size(), buffer, CppCommon::countof(buffer)));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Append the HTTP request body
    _cache.append(body);
    _body_index = index;
    _body_size = body.size();
    _body_length = body.size();
    _body_length_provided = true;
    return *this;
}

HTTPRequest& HTTPRequest::SetBodyLength(size_t length)
{
    // Append content length header
    char buffer[32];
    SetHeader("Content-Length", FastConvert(length, buffer, CppCommon::countof(buffer)));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Clear the HTTP request body
    _body_index = index;
    _body_size = 0;
    _body_length = length;
    _body_length_provided = true;
    return *this;
}

HTTPRequest& HTTPRequest::MakeHeadRequest(std::string_view url)
{
    Clear();
    SetBegin("HEAD", url);
    SetBody();
    return *this;
}

HTTPRequest& HTTPRequest::MakeGetRequest(std::string_view url)
{
    Clear();
    SetBegin("GET", url);
    SetBody();
    return *this;
}

HTTPRequest& HTTPRequest::MakePostRequest(std::string_view url, std::string_view content, std::string_view content_type)
{
    Clear();
    SetBegin("POST", url);
    if (!content_type.empty())
        SetHeader("Content-Type", content_type);
    SetBody(content);
    return *this;
}

HTTPRequest& HTTPRequest::MakePutRequest(std::string_view url, std::string_view content, std::string_view content_type)
{
    Clear();
    SetBegin("PUT", url);
    if (!content_type.empty())
        SetHeader("Content-Type", content_type);
    SetBody(content);
    return *this;
}

HTTPRequest& HTTPRequest::MakeDeleteRequest(std::string_view url)
{
    Clear();
    SetBegin("DELETE", url);
    SetBody();
    return *this;
}

HTTPRequest& HTTPRequest::MakeOptionsRequest(std::string_view url)
{
    Clear();
    SetBegin("OPTIONS", url);
    SetBody();
    return *this;
}

HTTPRequest& HTTPRequest::MakeTraceRequest(std::string_view url)
{
    Clear();
    SetBegin("TRACE", url);
    SetBody();
    return *this;
}

bool HTTPRequest::IsPendingHeader() const
{
    return (!_error && (_body_index == 0));
}

bool HTTPRequest::IsPendingBody() const
{
    return (!_error && (_body_index > 0) && (_body_size > 0));
}

bool HTTPRequest::ReceiveHeader(const void* buffer, size_t size)
{
    // Update the request cache
    _cache.insert(_cache.end(), (const char*)buffer, (const char*)buffer + size);

    // Try to seek for HTTP header separator
    for (size_t i = _cache_size; i < _cache.size(); ++i)
    {
        // Check for the request cache out of bounds
        if ((i + 3) >= _cache.size())
            break;

        // Check for the header separator
        if ((_cache[i + 0] == '\r') && (_cache[i + 1] == '\n') && (_cache[i + 2] == '\r') && (_cache[i + 3] == '\n'))
        {
            size_t index = 0;

            // Set the error flag for a while...
            _error = true;

            // Parse method
            _method_index = index;
            _method_size = 0;
            while (_cache[index] != ' ')
            {
                ++_method_size;
                ++index;
                if (index >= _cache.size())
                    return false;
            }
            ++index;
            if (index >= _cache.size())
                return false;

            // Parse URL
            _url_index = index;
            _url_size = 0;
            while (_cache[index] != ' ')
            {
                ++_url_size;
                ++index;
                if (index >= _cache.size())
                    return false;
            }
            ++index;
            if (index >= _cache.size())
                return false;

            // Parse protocol version
            _protocol_index = index;
            _protocol_size = 0;
            while (_cache[index] != '\r')
            {
                ++_protocol_size;
                ++index;
                if (index >= _cache.size())
                    return false;
            }
            ++index;
            if ((index >= _cache.size()) || (_cache[index] != '\n'))
                return false;
            ++index;
            if (index >= _cache.size())
                return false;

            // Parse headers
            while ((index < _cache.size()) && (index < i))
            {
                // Parse header name
                size_t header_name_index = index;
                size_t header_name_size = 0;
                while (_cache[index] != ':')
                {
                    ++header_name_size;
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= _cache.size())
                        return false;
                }
                ++index;
                if (index >= i)
                    break;
                if (index >= _cache.size())
                    return false;

                // Skip all prefix space characters
                while (std::isspace(_cache[index]))
                {
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= _cache.size())
                        return false;
                }

                // Parse header value
                size_t header_value_index = index;
                size_t header_value_size = 0;
                while (_cache[index] != '\r')
                {
                    ++header_value_size;
                    ++index;
                    if (index >= i)
                        break;
                    if (index >= _cache.size())
                        return false;
                }
                ++index;
                if ((index >= _cache.size()) || (_cache[index] != '\n'))
                    return false;
                ++index;
                if (index >= _cache.size())
                    return false;

                // Validate header name and value (sometimes value can be empty)
                if (header_name_size == 0)
                    return false;

                // Add a new header
                _headers.emplace_back(header_name_index, header_name_size, header_value_index, header_value_size);

                // Try to find the body content length
                if (CppCommon::StringUtils::CompareNoCase(std::string_view(_cache.data() + header_name_index, header_name_size), "Content-Length"))
                {
                    _body_length = 0;
                    for (size_t j = header_value_index; j < (header_value_index + header_value_size); ++j)
                    {
                        if ((_cache[j] < '0') || (_cache[j] > '9'))
                            return false;
                        _body_length *= 10;
                        _body_length += _cache[j] - '0';
                        _body_length_provided = true;
                    }
                }

                // Try to find Cookies
                if (CppCommon::StringUtils::CompareNoCase(std::string_view(_cache.data() + header_name_index, header_name_size), "Cookie"))
                {
                    bool name = true;
                    bool token = false;
                    size_t current = header_value_index;
                    size_t name_index = index;
                    size_t name_size = 0;
                    size_t cookie_index = index;
                    size_t cookie_size = 0;
                    for (size_t j = header_value_index; j < (header_value_index + header_value_size); ++j)
                    {
                        if (_cache[j] == ' ')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }
                            }
                            token = false;
                            continue;
                        }
                        if (_cache[j] == '=')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }
                            }
                            token = false;
                            name = false;
                            continue;
                        }
                        if (_cache[j] == ';')
                        {
                            if (token)
                            {
                                if (name)
                                {
                                    name_index = current;
                                    name_size = j - current;
                                }
                                else
                                {
                                    cookie_index = current;
                                    cookie_size = j - current;
                                }

                                // Validate the cookie
                                if ((name_size > 0) && (cookie_size > 0))
                                {
                                    // Add the cookie to the corresponding collection
                                    _cookies.emplace_back(name_index, name_size, cookie_index, cookie_size);

                                    // Resset the current cookie values
                                    name_index = j;
                                    name_size = 0;
                                    cookie_index = j;
                                    cookie_size = 0;
                                }
                            }
                            token = false;
                            name = true;
                            continue;
                        }
                        if (!token)
                        {
                            current = j;
                            token = true;
                        }
                    }

                    // Process the last cookie
                    if (token)
                    {
                        if (name)
                        {
                            name_index = current;
                            name_size = header_value_index + header_value_size - current;
                        }
                        else
                        {
                            cookie_index = current;
                            cookie_size = header_value_index + header_value_size - current;
                        }

                        // Validate the cookie
                        if ((name_size > 0) && (cookie_size > 0))
                        {
                            // Add the cookie to the corresponding collection
                            _cookies.emplace_back(name_index, name_size, cookie_index, cookie_size);
                        }
                    }
                }
            }

            // Reset the error flag
            _error = false;

            // Update the body index and size
            _body_index = i + 4;
            _body_size = _cache.size() - i - 4;

            // Update the parsed cache size
            _cache_size = _cache.size();

            return true;
        }
    }

    // Update the parsed cache size
    _cache_size = (_cache.size() >= 3) ? (_cache.size() - 3) : 0;

    return false;
}

bool HTTPRequest::ReceiveBody(const void* buffer, size_t size)
{
    // Update HTTP request cache
    _cache.insert(_cache.end(), (const char*)buffer, (const char*)buffer + size);

    // Update the parsed cache size
    _cache_size = _cache.size();

    // Update body size
    _body_size += size;

    // Check if the body length was provided
    if (_body_length_provided)
    {
        // Was the body fully received?
        if (_body_size >= _body_length)
        {
            _body_size = _body_length;
            return true;
        }
    }
    else
    {
        // HEAD/GET/DELETE/OPTIONS/TRACE request might have no body
        if ((method() == "HEAD") || (method() == "GET") || (method() == "DELETE") || (method() == "OPTIONS") || (method() == "TRACE"))
        {
            _body_length = 0;
            _body_size = 0;
            return true;
        }

        // Check the body content to find the request body end
        if (_body_size >= 4)
        {
            size_t index = _body_index + _body_size - 4;

            // Was the body fully received?
            if ((_cache[index + 0] == '\r') && (_cache[index + 1] == '\n') && (_cache[index + 2] == '\r') && (_cache[index + 3] == '\n'))
            {
                _body_length = _body_size;
                return true;
            }
        }
    }

    // Body was received partially...
    return false;
}

std::string_view HTTPRequest::FastConvert(size_t value, char* buffer, size_t size)
{
    size_t index = size;
    do
    {
        buffer[--index] = '0' + (value % 10);
        value /= 10;
    }
    while (value > 0);
    return std::string_view(buffer + index, size - index);
}

std::ostream& operator<<(std::ostream& os, const HTTPRequest& request)
{
    os << "Request method: " << request.method() << std::endl;
    os << "Request URL: " << request.url() << std::endl;
    os << "Request protocol: " << request.protocol() << std::endl;
    os << "Request headers: " << request.headers() << std::endl;
    for (size_t i = 0; i < request.headers(); ++i)
    {
        auto header = request.header(i);
        os << std::get<0>(header) << ": " << std::get<1>(header) << std::endl;
    }
    os << "Request body:" << request.body_length() << std::endl;
    os << request.body() << std::endl;
    return os;
}

void HTTPRequest::swap(HTTPRequest& request) noexcept
{
    using std::swap;
    swap(_error, request._error);
    swap(_method_index, request._method_index);
    swap(_method_size, request._method_size);
    swap(_url_index, request._url_index);
    swap(_url_size, request._url_size);
    swap(_protocol_index, request._protocol_index);
    swap(_protocol_size, request._protocol_size);
    swap(_headers, request._headers);
    swap(_cookies, request._cookies);
    swap(_body_index, request._body_index);
    swap(_body_size, request._body_size);
    swap(_body_length, request._body_length);
    swap(_body_length_provided, request._body_length_provided);
    swap(_cache, request._cache);
    swap(_cache_size, request._cache_size);
}

} // namespace HTTP
} // namespace CppServer
