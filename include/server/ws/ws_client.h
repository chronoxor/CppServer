/*!
    \file ws_client.h
    \brief WebSocket client definition
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WS_CLIENT_H
#define CPPSERVER_HTTP_WS_CLIENT_H

#include "server/http/http_client.h"
#include "server/ws/ws.h"

namespace CppServer {
namespace WS {

//! WebSocket client
/*!
    WebSocket client is used to communicate with WebSocket server.

    https://en.wikipedia.org/wiki/WebSocket

    Thread-safe.
*/
class WSClient : public HTTP::HTTPClient, protected WebSocket
{
public:
    using HTTPClient::HTTPClient;

    WSClient(const WSClient&) = delete;
    WSClient(WSClient&&) = delete;
    virtual ~WSClient() = default;

    WSClient& operator=(const WSClient&) = delete;
    WSClient& operator=(WSClient&&) = delete;

    // WebSocket connection methods
    bool Connect() override;
    bool Connect(const std::shared_ptr<Asio::TCPResolver>& resolver) override;
    bool ConnectAsync() override;
    bool ConnectAsync(const std::shared_ptr<Asio::TCPResolver>& resolver) override;
    virtual bool Close(int status) { SendClose(status, nullptr, 0); HTTPClient::Disconnect(); return true; }
    virtual bool CloseAsync(int status) { SendCloseAsync(status, nullptr, 0); HTTPClient::DisconnectAsync(); return true; }

    // WebSocket send text methods
    size_t SendText(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendText(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendTextAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendTextAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_TEXT, true, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket send binary methods
    size_t SendBinary(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendBinary(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendBinaryAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendBinaryAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_BINARY, true, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket close methods
    size_t SendClose(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, buffer, size, status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, text.data(), text.size(), status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, buffer, size, status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendClose(int status, std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, text.data(), text.size(), status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendCloseAsync(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, buffer, size, status); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendCloseAsync(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_CLOSE, true, text.data(), text.size(), status); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket ping methods
    size_t SendPing(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPing(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPingAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPingAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PING, true, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket pong methods
    size_t SendPong(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPong(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPongAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPongAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareSendFrame(WS_FIN | WS_PONG, true, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket receive methods
    std::string ReceiveText();
    std::string ReceiveText(const CppCommon::Timespan& timeout);
    std::vector<uint8_t> ReceiveBinary();
    std::vector<uint8_t> ReceiveBinary(const CppCommon::Timespan& timeout);

protected:
    void onConnected() override;
    void onDisconnected() override;
    void onReceived(const void* buffer, size_t size) override;
    void onReceivedResponseHeader(const HTTP::HTTPResponse& response) override;
    void onReceivedResponse(const HTTP::HTTPResponse& response) override;
    void onReceivedResponseError(const HTTP::HTTPResponse& response, const std::string& error) override;

    //! Handle WebSocket close notification
    void onWSClose(const void* buffer, size_t size) override { CloseAsync(1000); }
    //! Handle WebSocket ping notification
    void onWSPing(const void* buffer, size_t size) override { SendPongAsync(buffer, size); }
    //! Handle WebSocket error notification
    void onWSError(const std::string& message) override { onError(asio::error::fault, "WebSocket error", message); }

private:
    // Sync connect flag
    bool _sync_connect;

    // WebSocket clients cannot send response
    void SendResponse(const HTTP::HTTPResponse& response) override {}
};

/*! \example ws_chat_client.cpp WebSocket chat client example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_CLIENT_H
