/*!
    \file ws_session.h
    \brief WebSocket session definition
    \author Ivan Shynkarenka
    \date 27.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WS_SESSION_H
#define CPPSERVER_HTTP_WS_SESSION_H

#include "server/http/http_session.h"
#include "server/ws/ws.h"

namespace CppServer {
namespace WS {

class WSServer;

//! WebSocket session
/*!
    WebSocket session is used to read and write data from the connected WebSocket client.

    Thread-safe.
*/
class WSSession : public HTTP::HTTPSession, protected WebSocket
{
public:
    explicit WSSession(std::shared_ptr<WSServer> server);
    WSSession(const WSSession&) = delete;
    WSSession(WSSession&&) = delete;
    virtual ~WSSession() = default;

    WSSession& operator=(const WSSession&) = delete;
    WSSession& operator=(WSSession&&) = delete;

protected:
    void onDisconnected() override;
    void onReceived(const void* buffer, size_t size) override;
    void onReceivedRequestHeader(const HTTP::HTTPRequest& request) override;
    void onReceivedRequest(const HTTP::HTTPRequest& request) override;
    void onReceivedRequestError(const HTTP::HTTPRequest& request, const std::string& error) override { onError(asio::error::fault, "WebSocket error", error); }

    //! Handle WebSocket error notification
    void onWSError(const std::string& message) override { onError(asio::error::fault, "WebSocket error", message); }
};

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_SESSION_H
