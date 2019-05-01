/*!
    \file https_server.h
    \brief HTTPS server definition
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_HTTPS_SERVER_H
#define CPPSERVER_HTTP_HTTPS_SERVER_H

#include "https_session.h"

#include "server/asio/ssl_server.h"

namespace CppServer {
namespace HTTP {

//! HTTPS server
/*!
    HTTPS server is used to create secured HTTPS Web server and
    communicate with clients using secure HTTPS protocol.
    It allows to receive GET, POST, PUT, DELETE requests and
    send HTTP responses.

    Thread-safe.
*/
class HTTPSServer : public Asio::SSLServer
{
public:
    using SSLServer::SSLServer;

    HTTPSServer(const HTTPSServer&) = delete;
    HTTPSServer(HTTPSServer&&) = delete;
    virtual ~HTTPSServer() = default;

    HTTPSServer& operator=(const HTTPSServer&) = delete;
    HTTPSServer& operator=(HTTPSServer&&) = delete;

protected:
    std::shared_ptr<Asio::SSLSession> CreateSession(std::shared_ptr<Asio::SSLServer> server) override { return std::make_shared<HTTPSSession>(server); }
};

/*! \example https_server.cpp HTTPS server example */

} // namespace HTTP
} // namespace CppServer

#endif // CPPSERVER_HTTP_HTTPS_SERVER_H
