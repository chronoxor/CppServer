/*!
    \file wss_client.h
    \brief WebSocket secure client definition
    \author Ivan Shynkarenka
    \date 23.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_HTTP_WSS_CLIENT_H
#define CPPSERVER_HTTP_WSS_CLIENT_H

#include "server/http/https_client.h"

namespace CppServer {
namespace WS {

//! WebSocket secure client
/*!
    WebSocket secure client is used to communicate with secure WebSocket server.

    Thread-safe.
*/
class WSSClient : public HTTP::HTTPSClient
{
public:
    using HTTPSClient::HTTPSClient;

    WSSClient(const WSSClient&) = delete;
    WSSClient(WSSClient&&) = delete;
    virtual ~WSSClient() = default;

    WSSClient& operator=(const WSSClient&) = delete;
    WSSClient& operator=(WSSClient&&) = delete;

    bool Connect() override;
    bool Connect(std::shared_ptr<Asio::TCPResolver> resolver) override;
    bool ConnectAsync() override;
    bool ConnectAsync(std::shared_ptr<Asio::TCPResolver> resolver) override;

protected:
    void onHandshaked() override;
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
    virtual void onWSConnected(const HTTP::HTTPResponse& response) {}
    //! Handle WebSocket client disconnected notification
    virtual void onWSDisconnected() {}

private:
    // Handshaked flag
    bool _handshaked{false};
    // Sync connect flag
    bool _sync_connect;
};

/*! \example wss_client.cpp WebSocket secure client example */

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_HTTP_WSS_CLIENT_H
