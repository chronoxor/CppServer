/*!
    \file http_request.cpp
    \brief HTTP request implementation
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

#include "server/http/http_request.h"

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

void HTTPRequest::Clear()
{
    _error = false;
    _method_index = 0;
    _method_size = 0;
    _url_index = 0;
    _url_size = 0;
    _protocol_index = 0;
    _protocol_size = 0;
    _headers.clear();
    _body_index = 0;
    _body_size = 0;
    _body_length = 0;

    _cache.clear();
    _cache_size = 0;
}

void HTTPRequest::SetBegin(std::string_view method, std::string_view url, std::string_view protocol)
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
}

void HTTPRequest::SetHeader(std::string_view key, std::string_view value)
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
}

void HTTPRequest::SetBody(std::string_view body)
{
    // Append content length header
    SetHeader("Content-Length", std::to_string(body.size()));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Append the HTTP request body
    _cache.append(body);
    _body_index = index;
    _body_size = body.size();
    _body_length = body.size();
}

void HTTPRequest::SetBodyLength(size_t length)
{
    // Append content length header
    SetHeader("Content-Length", std::to_string(length));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Clear the HTTP request body
    _body_index = index;
    _body_size = 0;
    _body_length = length;
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

                // Validate header name and value
                if ((header_name_size == 0) || (header_value_size == 0))
                    return false;

                // Add a new header
                _headers.emplace_back(header_name_index, header_name_size, header_value_index, header_value_size);

                // Try to find the body content length
                if (std::string_view(_cache.data() + header_name_index, header_name_size) == "Content-Length")
                {
                    _body_length = 0;
                    for (size_t j = header_value_index; j < (header_value_index + header_value_size); ++j)
                    {
                        if ((_cache[j] < '0') || (_cache[j] > '9'))
                            return false;
                        _body_length *= 10;
                        _body_length += _cache[j] - '0';
                    }
                }
            }

            // Reset the error flag
            _error = false;

            // Update the body index and size
            _body_index = i + 4;
            _body_size = _cache.size() - i;

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

    // GET request has no body
    if ((method() == "HEAD") || (method() == "GET") || (method() == "TRACE"))
    {
        _body_length = 0;
        _body_size = 0;
        return true;
    }

    // Check if the body was fully parsed
    if ((_body_length > 0) && (_body_size >= _body_length))
    {
        _body_size = _body_length;
        return true;
    }

    return false;
}

} // namespace HTTP
} // namespace CppServer
