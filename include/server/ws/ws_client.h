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

namespace CppServer {
namespace WS {

//! WebSocket client
/*!
    WebSocket client is used to communicate with WebSocket server.

    Thread-safe.
*/
class WSClient : public HTTP::HTTPClient
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
    bool Connect(std::shared_ptr<Asio::TCPResolver> resolver) override;
    bool ConnectAsync() override;
    bool ConnectAsync(std::shared_ptr<Asio::TCPResolver> resolver) override;
    bool Disconnect() override { SendClose(nullptr, 0); return HTTPClient::Disconnect(); }
    bool DisconnectAsync() override { SendCloseAsync(nullptr, 0); return HTTPClient::DisconnectAsync(); }

    // WebSocket send text methods
    size_t SendText(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendText(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendTextAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendTextAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x1, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket send binary methods
    size_t SendBinary(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendBinary(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendBinaryAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendBinaryAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x2, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket close methods
    size_t SendClose(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendClose(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendCloseAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendCloseAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket ping methods
    size_t SendPing(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPing(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPingAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPingAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x9, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket pong methods
    size_t SendPong(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPong(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPongAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPongAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0xA, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

protected:
    void onConnected() override;
    void onDisconnected() override;
    void onReceived(const void* buffer, size_t size) override;
    void onReceivedResponseHeader(const HTTP::HTTPResponse& response) override;

    //! Handle WebSocket client connecting notification
    /*!
        Notification is called when WebSocket client is connecting
        to the server. You can handle the connection and change
        WebSocket upgrade HTTP request by providing your own headers.

        \param request - WebSocket upgrade HTTP request
    */
    virtual void onWSConnecting(HTTP::HTTPRequest& request) {}
    //! Handle WebSocket client connected notification
    /*!
        \param response - WebSocket upgrade HTTP response
    */
    virtual void onWSConnected(const HTTP::HTTPResponse& response) {}
    //! Handle WebSocket client disconnected notification
    virtual void onWSDisconnected() {}

    //! Handle WebSocket ping notification
    virtual void onWSPing() {}
    //! Handle WebSocket pong notification
    virtual void onWSPong() {}

private:
    // Handshaked flag
    bool _handshaked{false};
    // Sync connect flag
    bool _sync_connect;
    // Random mask
    uint32_t _mask;

    // WebSocket send buffer
    std::mutex _ws_send_lock;
    std::vector<uint8_t> _ws_send_buffer;

    void PrepareWebSocketFrame(uint32_t opcode, const void* buffer, size_t size);
};

/*! \example ws_client.cpp WebSocket client example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_CLIENT_H
