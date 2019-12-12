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
class WSServer : public HTTP::HTTPServer, protected WebSocket
{
public:
    using HTTPServer::HTTPServer;

    WSServer(const WSServer&) = delete;
    WSServer(WSServer&&) = delete;
    virtual ~WSServer() = default;

    WSServer& operator=(const WSServer&) = delete;
    WSServer& operator=(WSServer&&) = delete;

    // WebSocket connection methods
    virtual bool CloseAll(int status);

    //! Multicast data to all connected WebSocket sessions
    bool Multicast(const void* buffer, size_t size) override;

    // WebSocket multicast text methods
    size_t MulticastText(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, buffer, size); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t MulticastText(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, text.data(), text.size()); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket multicast binary methods
    size_t MulticastBinary(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, buffer, size); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t MulticastBinary(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, text.data(), text.size()); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket multicast ping methods
    size_t MulticastPing(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, buffer, size); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t MulticastPing(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, text.data(), text.size()); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket multicast pong methods
    size_t MulticastPong(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, buffer, size); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t MulticastPong(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, text.data(), text.size()); return Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()); }

protected:
    std::shared_ptr<Asio::TCPSession> CreateSession(const std::shared_ptr<Asio::TCPServer>& server) override { return std::make_shared<WSSession>(std::dynamic_pointer_cast<WSServer>(server)); }
};

/*! \example ws_chat_server.cpp WebSocket chat server example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_SERVER_H
