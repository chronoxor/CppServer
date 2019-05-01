/*!
    \file http_response.cpp
    \brief HTTP response implementation
    \author Ivan Shynkarenka
    \date 15.02.2019
    \copyright MIT License
*/

#include "server/http/http_response.h"

#include <cassert>

namespace CppServer {
namespace HTTP {

std::tuple<std::string_view, std::string_view> HTTPResponse::header(size_t i) const noexcept
{
    assert((i < _headers.size()) && "Index out of bounds!");
    if (i >= _headers.size())
        return std::make_tuple(std::string_view(), std::string_view());

    auto item = _headers[i];

    return std::make_tuple(std::string_view(_cache.data() + std::get<0>(item), std::get<1>(item)), std::string_view(_cache.data() + std::get<2>(item), std::get<3>(item)));
}

void HTTPResponse::Clear()
{
    _error = false;
    _status = 0;
    _status_phrase_index = 0;
    _status_phrase_size = 0;
    _protocol_index = 0;
    _protocol_size = 0;
    _headers.clear();
    _body_index = 0;
    _body_size = 0;
    _body_length = 0;

    _cache.clear();
    _cache_size = 0;
}

void HTTPResponse::SetBegin(int status, std::string_view protocol)
{
    std::string status_phrase;

    switch (status)
    {
        case 100: status_phrase = "Continue"; break;
        case 101: status_phrase = "Switching Protocols"; break;
        case 102: status_phrase = "Processing"; break;
        case 103: status_phrase = "Early Hints"; break;

        case 200: status_phrase = "OK"; break;
        case 201: status_phrase = "Created"; break;
        case 202: status_phrase = "Accepted"; break;
        case 203: status_phrase = "Non-Authoritative Information"; break;
        case 204: status_phrase = "No Content"; break;
        case 205: status_phrase = "Reset Content"; break;
        case 206: status_phrase = "Partial Content"; break;
        case 207: status_phrase = "Multi-Status"; break;
        case 208: status_phrase = "Already Reported"; break;

        case 226: status_phrase = "IM Used"; break;

        case 300: status_phrase = "Multiple Choices"; break;
        case 301: status_phrase = "Moved Permanently"; break;
        case 302: status_phrase = "Found"; break;
        case 303: status_phrase = "See Other"; break;
        case 304: status_phrase = "Not Modified"; break;
        case 305: status_phrase = "Use Proxy"; break;
        case 306: status_phrase = "Switch Proxy"; break;
        case 307: status_phrase = "Temporary Redirect"; break;
        case 308: status_phrase = "Permanent Redirect"; break;

        case 400: status_phrase = "Bad Request"; break;
        case 401: status_phrase = "Unauthorized"; break;
        case 402: status_phrase = "Payment Required"; break;
        case 403: status_phrase = "Forbidden"; break;
        case 404: status_phrase = "Not Found"; break;
        case 405: status_phrase = "Method Not Allowed"; break;
        case 406: status_phrase = "Not Acceptable"; break;
        case 407: status_phrase = "Proxy Authentication Required"; break;
        case 408: status_phrase = "Request Timeout"; break;
        case 409: status_phrase = "Conflict"; break;
        case 410: status_phrase = "Gone"; break;
        case 411: status_phrase = "Length Required"; break;
        case 412: status_phrase = "Precondition Failed"; break;
        case 413: status_phrase = "Payload Too Large"; break;
        case 414: status_phrase = "URI Too Long"; break;
        case 415: status_phrase = "Unsupported Media Type"; break;
        case 416: status_phrase = "Range Not Satisfiable"; break;
        case 417: status_phrase = "Expectation Failed"; break;

        case 421: status_phrase = "Misdirected Request"; break;
        case 422: status_phrase = "Unprocessable Entity"; break;
        case 423: status_phrase = "Locked"; break;
        case 424: status_phrase = "Failed Dependency"; break;
        case 425: status_phrase = "Too Early"; break;
        case 426: status_phrase = "Upgrade Required"; break;
        case 427: status_phrase = "Unassigned"; break;
        case 428: status_phrase = "Precondition Required"; break;
        case 429: status_phrase = "Too Many Requests"; break;
        case 431: status_phrase = "Request Header Fields Too Large"; break;

        case 451: status_phrase = "Unavailable For Legal Reasons"; break;

        case 500: status_phrase = "Internal Server Error"; break;
        case 501: status_phrase = "Not Implemented"; break;
        case 502: status_phrase = "Bad Gateway"; break;
        case 503: status_phrase = "Service Unavailable"; break;
        case 504: status_phrase = "Gateway Timeout"; break;
        case 505: status_phrase = "HTTP Version Not Supported"; break;
        case 506: status_phrase = "Variant Also Negotiates"; break;
        case 507: status_phrase = "Insufficient Storage"; break;
        case 508: status_phrase = "Loop Detected"; break;

        case 510: status_phrase = "Not Extended"; break;
        case 511: status_phrase = "Network Authentication Required"; break;

        default: status_phrase = "Unknown"; break;
    }

    SetBegin(status, status_phrase, protocol);
}

void HTTPResponse::SetBegin(int status, std::string_view status_phrase, std::string_view protocol)
{
    // Clear the HTTP response cache
    Clear();

    size_t index = 0;

    // Append the HTTP response protocol version
    _cache.append(protocol);
    _protocol_index = index;
    _protocol_size = protocol.size();

    _cache.append(" ");
    index = _cache.size();

    // Append the HTTP response status
    _cache.append(std::to_string(status));
    _status = status;

    _cache.append(" ");
    index = _cache.size();

    // Append the HTTP response status phrase
    _cache.append(status_phrase);
    _status_phrase_index = index;
    _status_phrase_size = status_phrase.size();

    _cache.append("\r\n");
}

void HTTPResponse::SetHeader(std::string_view key, std::string_view value)
{
    size_t index = _cache.size();

    // Append the HTTP response header's key
    _cache.append(key);
    size_t key_index = index;
    size_t key_size = key.size();

    _cache.append(": ");
    index = _cache.size();

    // Append the HTTP response header's value
    _cache.append(value);
    size_t value_index = index;
    size_t value_size = value.size();

    _cache.append("\r\n");

    // Add the header to the corresponding collection
    _headers.emplace_back(key_index, key_size, value_index, value_size);
}

void HTTPResponse::SetBody(std::string_view body)
{
    // Append non empty content length header
    SetHeader("Content-Length", std::to_string(body.size()));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Append the HTTP response body
    _cache.append(body);
    _body_index = index;
    _body_size = body.size();
    _body_length = body.size();
}

void HTTPResponse::SetBodyLength(size_t length)
{
    // Append content length header
    SetHeader("Content-Length", std::to_string(length));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Clear the HTTP response body
    _body_index = index;
    _body_size = 0;
    _body_length = length;
}

bool HTTPResponse::IsPendingHeader() const
{
    return (!_error && (_body_index == 0));
}

bool HTTPResponse::IsPendingBody() const
{
    return (!_error && (_body_index > 0) && (_body_size > 0));
}

bool HTTPResponse::ReceiveHeader(const void* buffer, size_t size)
{
    // Update the response cache
    _cache.insert(_cache.end(), (const char*)buffer, (const char*)buffer + size);

    // Try to seek for HTTP header separator
    for (size_t i = _cache_size; i < _cache.size(); ++i)
    {
        // Check for the response cache out of bounds
        if ((i + 3) >= _cache.size())
            break;

        // Check for the header separator
        if ((_cache[i + 0] == '\r') && (_cache[i + 1] == '\n') && (_cache[i + 2] == '\r') && (_cache[i + 3] == '\n'))
        {
            size_t index = 0;

            // Set the error flag for a while...
            _error = true;

            // Parse protocol version
            _protocol_index = index;
            _protocol_size = 0;
            while (_cache[index] != ' ')
            {
                ++_protocol_size;
                ++index;
                if (index >= _cache.size())
                    return false;
            }
            ++index;
            if (index >= _cache.size())
                return false;

            // Parse status code
            size_t status_index = index;
            size_t status_size = 0;
            while (_cache[index] != ' ')
            {
                if ((_cache[index] < '0') || (_cache[index] > '9'))
                    return false;
                ++status_size;
                ++index;
                if (index >= _cache.size())
                    return false;
            }
            _status = 0;
            for (size_t j = status_index; j < (status_index + status_size); ++j)
            {
                _status *= 10;
                _status += _cache[j] - '0';
            }
            ++index;
            if (index >= _cache.size())
                return false;

            // Parse status phrase
            _status_phrase_index = index;
            _status_phrase_size = 0;
            while (_cache[index] != '\r')
            {
                ++_status_phrase_size;
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

bool HTTPResponse::ReceiveBody(const void* buffer, size_t size)
{
    // Update HTTP response cache
    _cache.insert(_cache.end(), (const char*)buffer, (const char*)buffer + size);

    // Update the parsed cache size
    _cache_size = _cache.size();

    // Update body size
    _body_size += size;

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
