/*!
    \file wss_session.h
    \brief WebSocket secure session definition
    \author Ivan Shynkarenka
    \date 28.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WSS_SESSION_H
#define CPPSERVER_HTTP_WSS_SESSION_H

#include "server/http/https_session.h"
#include "server/ws/ws.h"

namespace CppServer {
namespace WS {

class WSSServer;

//! WebSocket secure session
/*!
    WebSocket secure session is used to read and write data from the connected WebSocket secure client.

    Thread-safe.
*/
class WSSSession : public HTTP::HTTPSSession, protected WebSocket
{
    friend class WSSServer;

public:
    explicit WSSSession(const std::shared_ptr<WSSServer>& server);
    WSSSession(const WSSSession&) = delete;
    WSSSession(WSSSession&&) = delete;
    virtual ~WSSSession() = default;

    WSSSession& operator=(const WSSSession&) = delete;
    WSSSession& operator=(WSSSession&&) = delete;

    // WebSocket connection methods
    virtual bool Close(int status) { SendCloseAsync(status, nullptr, 0); HTTPSSession::Disconnect(); return true; }

    // WebSocket send text methods
    size_t SendText(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendText(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendTextAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, buffer, size); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendTextAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, false, text.data(), text.size()); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket send binary methods
    size_t SendBinary(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendBinary(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendBinaryAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, buffer, size); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendBinaryAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, false, text.data(), text.size()); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket close methods
    size_t SendClose(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, buffer, size, status); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, text.data(), text.size(), status); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, buffer, size, status); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendClose(int status, std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, text.data(), text.size(), status); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendCloseAsync(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, buffer, size, status); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendCloseAsync(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, false, text.data(), text.size(), status); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket ping methods
    size_t SendPing(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPing(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPingAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, buffer, size); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPingAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, false, text.data(), text.size()); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket pong methods
    size_t SendPong(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, buffer, size); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPong(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, text.data(), text.size()); return HTTPSSession::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPongAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, buffer, size); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPongAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, false, text.data(), text.size()); return HTTPSSession::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket receive methods
    std::string ReceiveText();
    std::string ReceiveText(const CppCommon::Timespan& timeout);
    std::vector<uint8_t> ReceiveBinary();
    std::vector<uint8_t> ReceiveBinary(const CppCommon::Timespan& timeout);

protected:
    void onDisconnected() override;
    void onReceived(const void* buffer, size_t size) override;
    void onReceivedRequestHeader(const HTTP::HTTPRequest& request) override;
    void onReceivedRequest(const HTTP::HTTPRequest& request) override;
    void onReceivedRequestError(const HTTP::HTTPRequest& request, const std::string& error) override;

    //! Handle WebSocket close notification
    void onWSClose(const void* buffer, size_t size) override { Close(1000); }
    //! Handle WebSocket ping notification
    void onWSPing(const void* buffer, size_t size) override { SendPongAsync(buffer, size); }
    //! Handle WebSocket error notification
    void onWSError(const std::string& message) override { onError(asio::error::fault, "WebSocket error", message); }

private:
    // WebSocket send response
    void SendResponse(const HTTP::HTTPResponse& response) override { SendResponseAsync(response); }
};

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WSS_SESSION_H
