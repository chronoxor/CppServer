/*!
    \file wss_server.h
    \brief WebSocket secure server definition
    \author Ivan Shynkarenka
    \date 28.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WSS_SERVER_H
#define CPPSERVER_HTTP_WSS_SERVER_H

#include "wss_session.h"

#include "server/http/https_server.h"

namespace CppServer {
namespace WS {

//! WebSocket secure server
/*!
    WebSocket secure server is used to communicate with clients using
    WebSocket secure protocol.

    https://en.wikipedia.org/wiki/WebSocket

    Thread-safe.
*/
class WSSServer : public HTTP::HTTPSServer, protected WebSocket
{
public:
    using HTTPSServer::HTTPSServer;

    WSSServer(const WSSServer&) = delete;
    WSSServer(WSSServer&&) = delete;
    virtual ~WSSServer() = default;

    WSSServer& operator=(const WSSServer&) = delete;
    WSSServer& operator=(WSSServer&&) = delete;

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
    std::shared_ptr<Asio::SSLSession> CreateSession(const std::shared_ptr<Asio::SSLServer>& server) override { return std::make_shared<WSSSession>(std::dynamic_pointer_cast<WSSServer>(server)); }
};

/*! \example wss_chat_server.cpp WebSocket secure chat server example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WSS_SERVER_H
