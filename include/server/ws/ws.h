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
    //! Final frame
    static const uint8_t WS_FIN = 0x80;
    //! Text frame
    static const uint8_t WS_TEXT = 0x01;
    //! Binary frame
    static const uint8_t WS_BINARY = 0x02;
    //! Close frame
    static const uint8_t WS_CLOSE = 0x08;
    //! Ping frame
    static const uint8_t WS_PING = 0x09;
    //! Pong frame
    static const uint8_t WS_PONG = 0x0A;

    WebSocket() { ClearWSBuffers(); }
    WebSocket(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    ~WebSocket() = default;

    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

    //! Perform WebSocket client upgrade
    /*!
        \param response - WebSocket upgrade HTTP response
        \param id - WebSocket client Id
        \return 'true' if the WebSocket was successfully upgrade, 'false' if the WebSocket was not upgrade
    */
    bool PerformClientUpgrade(const HTTP::HTTPResponse& response, const CppCommon::UUID& id);

    //! Perform WebSocket server upgrade
    /*!
        \param request - WebSocket upgrade HTTP request
        \param response - WebSocket upgrade HTTP response
        \return 'true' if the WebSocket was successfully upgrade, 'false' if the WebSocket was not upgrade
    */
    bool PerformServerUpgrade(const HTTP::HTTPRequest& request, HTTP::HTTPResponse& response);

    //! Prepare WebSocket send frame
    /*!
        \param opcode - WebSocket opcode
        \param mask - WebSocket mask
        \param buffer - Buffer to send
        \param size - Buffer size
        \param status - WebSocket status (defualt is 0)
    */
    void PrepareSendFrame(uint8_t opcode, bool mask, const void* buffer, size_t size, int status = 0);

    //! Prepare WebSocket receive frame
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    void PrepareReceiveFrame(const void* buffer, size_t size);

    //! Required WebSocket receive frame size
    size_t RequiredReceiveFrameSize();

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
    //! Handle WebSocket client connected notification
    /*!
        \param response - WebSocket upgrade HTTP response
    */
    virtual void onWSConnected(const HTTP::HTTPResponse& response) {}

    //! Handle WebSocket server session validating notification
    /*!
        Notification is called when WebSocket client is connecting
        to the server. You can handle the connection and validate
        WebSocket upgrade HTTP request.

        \param request - WebSocket upgrade HTTP request
        \param response - WebSocket upgrade HTTP response
        \return 'true' if the WebSocket update request is valid, 'false' if the WebSocket update request is not valid
    */
    virtual bool onWSConnecting(const HTTP::HTTPRequest& request, HTTP::HTTPResponse& response) { return true; }
    //! Handle WebSocket server session connected notification
    /*!
        \param request - WebSocket upgrade HTTP request
    */
    virtual void onWSConnected(const HTTP::HTTPRequest& request) {}

    //! Handle WebSocket client disconnected notification
    virtual void onWSDisconnected() {}

    //! Handle WebSocket received notification
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onWSReceived(const void* buffer, size_t size) {}

    //! Handle WebSocket client close notification
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onWSClose(const void* buffer, size_t size) {}
    //! Handle WebSocket ping notification
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onWSPing(const void* buffer, size_t size) {}
    //! Handle WebSocket pong notification
    /*!
        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onWSPong(const void* buffer, size_t size) {}

    //! Handle WebSocket error notification
    /*!
        \param message - Error message
    */
    virtual void onWSError(const std::string& message) {}

protected:
    //! Handshaked flag
    bool _ws_handshaked{false};

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

    //! Send WebSocket server upgrade response
    /*!
        \param response - WebSocket upgrade HTTP response
    */
    virtual void SendResponse(const HTTP::HTTPResponse& response) {}
};

} // namespace WS
} // namespace CppServer

#endif // CPPSERVER_WS_H
