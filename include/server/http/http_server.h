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

protected:
    std::shared_ptr<Asio::TCPSession> CreateSession(std::shared_ptr<Asio::TCPServer> server) override { return std::make_shared<HTTPSession>(server); }
};

/*! \example http_server.cpp HTTP server example */

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTP_SERVER_H
