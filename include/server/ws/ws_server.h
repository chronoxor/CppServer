/*!
    \file ws_server.h
    \brief WebSocket server definition
    \author Ivan Shynkarenka
    \date 27.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WS_SERVER_H
#define CPPSERVER_HTTP_WS_SERVER_H

#include "ws_session.h"

#include "server/http/http_server.h"

namespace CppServer {
namespace WS {

//! WebSocket server
/*!
    WebSocket server is used to communicate with clients using
    WebSocket protocol.

    https://en.wikipedia.org/wiki/WebSocket

    Thread-safe.
*/
class WSServer : public HTTP::HTTPServer
{
public:
    using HTTPServer::HTTPServer;

    WSServer(const WSServer&) = delete;
    WSServer(WSServer&&) = delete;
    virtual ~WSServer() = default;

    WSServer& operator=(const WSServer&) = delete;
    WSServer& operator=(WSServer&&) = delete;

protected:
    std::shared_ptr<Asio::TCPSession> CreateSession(std::shared_ptr<Asio::TCPServer> server) override { return std::make_shared<WSSession>(std::dynamic_pointer_cast<WSServer>(server)); }
};

/*! \example ws_server.cpp WebSocket server example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_SERVER_H
