/*!
    \file ws.h
    \brief WebSocket C++ Library definition
    \author Ivan Shynkarenka
    \date 22.05.2019
    \copyright MIT License
*/

#ifndef CPPSERVER_WS_H
#define CPPSERVER_WS_H

#include "server/http/http_request.h"
#include "server/http/http_response.h"

#include "system/uuid.h"

#include <mutex>

namespace CppServer {

/*!
    \namespace CppServer::WS
    \brief WebSocket definitions
*/
namespace WS {

//! WebSocket utility class
class WebSocket
{
public:
    //! Text frame
    static const uint8_t WS_TEXT = 0x81;
    //! Binary frame
    static const uint8_t WS_BINARY = 0x82;
    //! Close frame
    static const uint8_t WS_CLOSE = 0x88;
    //! Ping frame
    static const uint8_t WS_PING = 0x89;
    //! Pong frame
    static const uint8_t WS_PONG = 0x8A;

    WebSocket() = default;
    WebSocket(const WebSocket&) = default;
    WebSocket(WebSocket&&) = default;
    ~WebSocket() = default;

    WebSocket& operator=(const WebSocket&) = default;
    WebSocket& operator=(WebSocket&&) = default;

    //! Perform WebSocket upgrade
    /*!
        \param response - WebSocket upgrade HTTP response
        \param id - WebSocket Id
        \return 'true' if the WebSocket was successfully upgrade, 'false' if the WebSocket was not upgrade
    */
    bool PerformUpgrade(const HTTP::HTTPResponse& response, const CppCommon::UUID& id);

    //! Prepare WebSocket send frame
    /*!
        \param opcode - WebSocket opcode
        \param buffer - Buffer to send
        \param size - Buffer size
        \param status - WebSocket status (defualt is 0)
    */
    void PrepareSendFrame(uint8_t opcode, const void* buffer, size_t size, int status = 0);

protected:
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

    //! Handle WebSocket error notification
    /*!
        \param message - Error message
    */
    virtual void onWSError(const std::string& message) {}

protected:
    // Handshaked flag
    bool _handshaked{false};

    //! Random mask
    uint8_t _mask[4];

    //! Send buffer lock
    std::mutex _ws_send_lock;
    //! Send buffer
    std::vector<uint8_t> _ws_send_buffer;
};

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_WS_H
