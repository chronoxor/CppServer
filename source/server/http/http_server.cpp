/*!
    \file http_server.cpp
    \brief HTTP server implementation
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#include "server/http/http_server.h"

#include "string/format.h"

namespace CppServer {
namespace HTTP {

void HTTPServer::AddStaticContent(const CppCommon::Path& path, const std::string& prefix, const CppCommon::Timespan& timeout)
{
    auto hanlder = [](CppCommon::FileCache & cache, const std::string& key, const std::string& value, const CppCommon::Timespan& timespan)
    {
        auto response = HTTPResponse();
        response.SetBegin(200);
        response.SetContentType(CppCommon::Path(key).extension().string());
        response.SetHeader("Cache-Control", CppCommon::format("max-age={}", timespan.seconds()));
        response.SetBody(value);
        return cache.insert(key, response.cache(), timespan);
    };

    cache().insert_path(path, prefix, timeout, hanlder);
}

} // namespace HTTP
} // namespace CppServer
