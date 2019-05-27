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
    WebSocket(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    ~WebSocket() = default;

    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

    //! Perform WebSocket upgrade on a server side
    /*!
        \param request - WebSocket upgrade HTTP request
        \return 'true' if the WebSocket was successfully upgrade, 'false' if the WebSocket was not upgrade
    */
    bool PerformUpgrade(const HTTP::HTTPRequest& request);
    //! Perform WebSocket upgrade on a client side
    /*!
        \param response - WebSocket upgrade HTTP response
        \param id - WebSocket client Id
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

    //! Required WebSocket receive frame size
    size_t RequiredReceiveFrameSize();

    //! Prepare WebSocket receive frame
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    void PrepareReceiveFrame(const void* buffer, size_t size);

    //! Clear WebSocket send/receive buffers
    void ClearWSBuffers();

protected:
    //! Handle WebSocket client connecting notification
    /*!
        Notification is called when WebSocket client is connecting
        to the server. You can handle the connection and change
        WebSocket upgrade HTTP request by providing your own headers.

        \param request - WebSocket upgrade HTTP request
    */
    virtual void onWSConnecting(HTTP::HTTPRequest& request) {}
    //! Handle WebSocket client validating notification
    /*!
        Notification is called when WebSocket client is connecting
        to the server. You can handle the connection and validate
        WebSocket upgrade HTTP request.

        \param request - WebSocket upgrade HTTP request
        \return 'true' if the WebSocket update request is valid, 'false' if the WebSocket update request is not valid
    */
    virtual bool onWSValidating(const HTTP::HTTPRequest& request) { return true; }
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

    //! Handle WebSocket received notification
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onWSReceived(const void* buffer, size_t size) {}

    //! Handle WebSocket error notification
    /*!
        \param message - Error message
    */
    virtual void onWSError(const std::string& message) {}

protected:
    // Handshaked flag
    bool _handshaked{false};

    //! Received frame flag
    bool _ws_received{false};
    //! Received frame header size
    size_t _ws_header_size{0};
    //! Received frame payload size
    size_t _ws_payload_size{0};
    //! Receive buffer
    std::vector<uint8_t> _ws_receive_buffer;
    //! Receive mask
    uint8_t _ws_receive_mask[4];

    //! Send buffer lock
    std::mutex _ws_send_lock;
    //! Send buffer
    std::vector<uint8_t> _ws_send_buffer;
    //! Send mask
    uint8_t _ws_send_mask[4];
};

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_WS_H
