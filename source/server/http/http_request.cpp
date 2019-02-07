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
    _method_index = 0;
    _method_size = 0;
    _url_index = 0;
    _url_size = 0;
    _protocol_index = 0;
    _protocol_size = 0;
    _headers.clear();
    _body_index = 0;
    _body_size = 0;

    _cache.clear();
}

void HTTPRequest::SetBegin(const std::string_view& method, const std::string_view& url, const std::string_view& protocol)
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

void HTTPRequest::SetHeader(const std::string_view& key, const std::string_view& value)
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

void HTTPRequest::SetBody(const std::string_view& body)
{
    // Append non empty content length header
    if (!body.empty())
        SetHeader("Content-Length", std::to_string(body.size()));

    _cache.append("\r\n");

    size_t index = _cache.size();

    // Append the HTTP request body
    _cache.append(body);
    _body_index = index;
    _body_size = body.size();
}

} // namespace HTTP
} // namespace CppServer
