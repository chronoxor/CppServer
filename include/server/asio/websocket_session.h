/*!
    \file websocket_session.h
    \brief WebSocket session definition
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEBSOCKET_SESSION_H
#define CPPSERVER_ASIO_WEBSOCKET_SESSION_H

#include "service.h"
#include "websocket.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class WebSocketServer;

//! WebSocket session
/*!
    WebSocketCP session is used to read and write data from the connected WebSocket client.

    Thread-safe.
*/
template <class TServer, class TSession>
class WebSocketSession : public std::enable_shared_from_this<WebSocketSession<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class WebSocketServer;

    typedef websocketpp::connection<websocketpp::config::asio> wsconnection;

public:
    //! Initialize the session with a given server
    /*!
        \param server - Connected server
    */
    explicit WebSocketSession(std::shared_ptr<WebSocketServer<TServer, TSession>> server);
    WebSocketSession(const WebSocketSession&) = delete;
    WebSocketSession(WebSocketSession&&) = default;
    virtual ~WebSocketSession() { Disconnect(); }

    WebSocketSession& operator=(const WebSocketSession&) = delete;
    WebSocketSession& operator=(WebSocketSession&&) = default;

    //! Get the session Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _server->service(); }
    //! Get the session server
    std::shared_ptr<WebSocketServer<TServer, TSession>>& server() noexcept { return _server; }
    //! Get the session connection
    websocketpp::connection_hdl& connection() noexcept { return _connection; }

    //! Is the session connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Disconnect the session
    /*!
        \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
    */
    bool Disconnect();

    //! Send data into the session
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of pending bytes in the send buffer
    */
    size_t Send(const void* buffer, size_t size);

protected:
    //! Handle session connected notification
    virtual void onConnected() {}
    //! Handle session disconnected notification
    virtual void onDisconnected() {}

    //! Handle buffer received notification
    /*!
        Notification is called when another chunk of buffer was received
        from the client.

        Default behavior is to handle all bytes from the received buffer.
        If you want to wait for some more bytes from the client return the
        size of the buffer you want to keep until another chunk is received.

        \param buffer - Received buffer
        \param size - Received buffer size
        \return Count of handled bytes
    */
    virtual size_t onReceived(const void* buffer, size_t size) { return size; }
    //! Handle buffer sent notification
    /*!
        Notification is called when another chunk of buffer was sent
        to the client.

        This handler could be used to send another buffer to the client
        for instance when the pending size is zero.

        \param sent - Size of sent buffer
        \param pending - Size of pending buffer
    */
    virtual void onSent(size_t sent, size_t pending) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Session Id
    CppCommon::UUID _id;
    // Session server & socket
    std::shared_ptr<WebSocketServer<TServer, TSession>> _server;
    websocketpp::connection_hdl _connection;
    std::atomic<bool> _connected;
    // Receive & send buffers
    std::mutex _send_lock;
    std::vector<wsconnection::message_ptr> _send_buffer;
    bool _reciving;
    bool _sending;

    static const size_t CHUNK = 8192;

    //! Connect the session
    /*!
        \param connection - WebSocket connection
    */
    void Connect(websocketpp::connection_hdl connection);

    //! Try to send pending data
    void TrySend();

    //! Clear receive & send buffers
    void ClearBuffers();
};

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEBSOCKET_SESSION_H
