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
    bool Disconnect() override { SendClose(1000, nullptr, 0); return HTTPClient::Disconnect(); }
    bool DisconnectAsync() override { SendCloseAsync(1000, nullptr, 0); return HTTPClient::DisconnectAsync(); }

    // WebSocket send text methods
    size_t SendText(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendText(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendText(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendTextAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendTextAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x81, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket send binary methods
    size_t SendBinary(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendBinary(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendBinary(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendBinaryAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendBinaryAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x82, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket close methods
    size_t SendClose(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, buffer, size, status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, text.data(), text.size(), status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendClose(int status, const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, buffer, size, status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendClose(int status, std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, text.data(), text.size(), status); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendCloseAsync(int status, const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, buffer, size, status); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendCloseAsync(int status, std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x88, text.data(), text.size(), status); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket ping methods
    size_t SendPing(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPing(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPing(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPingAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPingAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x89, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

    // WebSocket pong methods
    size_t SendPong(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    size_t SendPong(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, buffer, size); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    size_t SendPong(std::string_view text, const CppCommon::Timespan& timeout) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, text.data(), text.size()); return HTTPClient::Send(_ws_send_buffer.data(), _ws_send_buffer.size(), timeout); }
    bool SendPongAsync(const void* buffer, size_t size) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, buffer, size); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }
    bool SendPongAsync(std::string_view text) { std::scoped_lock locker(_ws_send_lock); PrepareWebSocketFrame(0x8A, text.data(), text.size()); return HTTPClient::SendAsync(_ws_send_buffer.data(), _ws_send_buffer.size()); }

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
    virtual void onWSConnecting(HTTP::HTTPRequest& request)
    {
        request.SetBegin("GET", "/");
        request.SetHeader("Upgrade", "websocket");
        request.SetHeader("Connection", "Upgrade");
        request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(id().string()));
        request.SetHeader("Sec-WebSocket-Protocol", "chat, superchat");
        request.SetHeader("Sec-WebSocket-Version", "13");
    }
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
    uint8_t _mask[4];

    // WebSocket send buffer
    std::mutex _ws_send_lock;
    std::vector<uint8_t> _ws_send_buffer;

    void PrepareWebSocketFrame(uint8_t opcode, const void* buffer, size_t size, int status = 0);
};

/*! \example ws_client.cpp WebSocket client example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WS_CLIENT_H
