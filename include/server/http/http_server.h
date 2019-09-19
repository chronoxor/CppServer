/*!
    \file http_server.h
    \brief HTTP server definition
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTP_SERVER_H
#define CPPSERVER_HTTP_HTTP_SERVER_H

#include "http_session.h"

#include "cache/filecache.h"
#include "server/asio/tcp_server.h"

namespace CppServer {
namespace HTTP {

//! HTTP server
/*!
    HTTP server is used to create HTTP Web server and
    communicate with clients using HTTP protocol.
    It allows to receive GET, POST, PUT, DELETE requests and
    send HTTP responses.

    Thread-safe.
*/
class HTTPServer : public Asio::TCPServer
{
public:
    using TCPServer::TCPServer;

    HTTPServer(const HTTPServer&) = delete;
    HTTPServer(HTTPServer&&) = delete;
    virtual ~HTTPServer() = default;

    HTTPServer& operator=(const HTTPServer&) = delete;
    HTTPServer& operator=(HTTPServer&&) = delete;

    //! Get the static content cache
    CppCommon::FileCache& cache() noexcept { return _cache; }
    const CppCommon::FileCache& cache() const noexcept { return _cache; }

    //! Add static content cache
    /*!
        \param path - Static content path
        \param prefix - Cache prefix (default is "/")
        \param timeout - Refresh cache timeout (default is 1 hour)
    */
    void AddStaticContent(const CppCommon::Path& path, const std::string& prefix = "/", const CppCommon::Timespan& timeout = CppCommon::Timespan::hours(1));
    //! Remove static content cache
    /*!
        \param path - Static content path
    */
    void RemoveStaticContent(const CppCommon::Path& path) { _cache.remove_path(path); }
    //! Clear static content cache
    void ClearStaticContent() { _cache.clear(); }

    //! Watchdog the static content cache
    void Watchdog(const CppCommon::UtcTimestamp& utc = CppCommon::UtcTimestamp()) { _cache.watchdog(utc); }

protected:
    std::shared_ptr<Asio::TCPSession> CreateSession(const std::shared_ptr<Asio::TCPServer>& server) override { return std::make_shared<HTTPSession>(std::dynamic_pointer_cast<HTTPServer>(server)); }

private:
    // Static content cache
    CppCommon::FileCache _cache;
};

/*! \example http_server.cpp HTTP server example */

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_SERVER_H
