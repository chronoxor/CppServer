/*!
    \file http_request.cpp
    \brief HTTP request implementation
    \author Ivan Shynkarenka
    \date 07.02.2019
    \copyright MIT License
*/

#include "server/http/http_request.h"

namespace CppServer {
namespace Http {

void HttpRequest::Clear()
{
    _cache.clear();
    _method = std::string_view();
    _url = std::string_view();
    _protocol = std::string_view();
    _headers.clear();
}

void HttpRequest::Set(const std::string_view& method, const std::string_view& url, const std::string_view& protocol)
{
    // Clear the current HTTP request
    Clear();

    size_t index = 0;

    // Append HTTP method
    _cache.append(method);
    _method = std::string_view(_cache.data() + index, method.size());

    _cache.append(" ");
    index = _cache.size();

    // Append request URL
    _cache.append(url);
    _url = std::string_view(_cache.data() + index, url.size());

    _cache.append(" ");
    index = _cache.size();

    // Append protocol version
    _cache.append(protocol);
    _protocol = std::string_view(_cache.data() + index, protocol.size());

    _cache.append("\r\n");
    index = _cache.size();
}

void HttpRequest::SetHeader(const std::string_view& key, const std::string_view& value)
{
    size_t index = _cache.size();

    std::string_view key_view;
    std::string_view value_view;

    // Append HTTP header's key
    _cache.append(key);
    key_view = std::string_view(_cache.data() + index, key.size());

    _cache.append(": ");
    index = _cache.size();

    // Append HTTP header's value
    _cache.append(value);
    value_view = std::string_view(_cache.data() + index, value.size());

    // Add HTTP header to the corresponding collection
    _headers.emplace(key_view, value_view);
}

} // namespace Http
} // namespace CppServer
