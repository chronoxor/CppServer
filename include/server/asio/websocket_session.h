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

public:
    //! Initialize the session with a given server
    /*!
        \param server - Connected server
    */
    explicit WebSocketSession(std::shared_ptr<WebSocketServer<TServer, TSession>> server);
    WebSocketSession(const WebSocketSession&) = delete;
    WebSocketSession(WebSocketSession&&) = default;
    virtual ~WebSocketSession() = default;

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

    //! Total bytes received
    size_t total_received() const noexcept { return _total_received; }
    //! Total bytes sent
    size_t total_sent() const noexcept { return _total_sent; }

    //! Is the session connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Disconnect the session
    /*!
        \param code - Close code to send (default is normal)
        \param reason - Close reason to send (default is "")
        \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
    */
    bool Disconnect(websocketpp::close::status::value code = websocketpp::close::status::normal, const std::string& reason = "") { return Disconnect(false, code, reason); }

    //! Send data into the session
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \param opcode - Data opcode (default is websocketpp::frame::opcode::binary)
        \return Count of sent bytes
    */
    size_t Send(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::binary);
    //! Send a text string into the session
    /*!
        \param text - Text string to send
        \param opcode - Data opcode (default is websocketpp::frame::opcode::text)
        \return Count of sent bytes
    */
    size_t Send(const std::string& text, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::text);
    //! Send a message into the session
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t Send(WebSocketMessage message);

protected:
    //! Handle session connected notification
    virtual void onConnected() {}
    //! Handle session disconnected notification
    virtual void onDisconnected() {}

    //! Handle message received notification
    /*!
        \param message - Received message
    */
    virtual void onReceived(WebSocketMessage message) {}

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
    // Session server & connection
    std::shared_ptr<WebSocketServer<TServer, TSession>> _server;
    websocketpp::connection_hdl _connection;
    std::atomic<bool> _connected;
    // Session statistic
    size_t _total_received;
    size_t _total_sent;

    //! Connect the session
    /*!
        \param connection - WebSocket connection
    */
    void Connect(websocketpp::connection_hdl connection);
    //! Disconnect the session
    /*!
        \param dispatch - Dispatch flag
        \param code - Close code to send (default is normal)
        \param reason - Close reason to send (default is "")
        \return 'true' if the session was successfully disconnected, 'false' if the session is already disconnected
    */
    bool Disconnect(bool dispatch, websocketpp::close::status::value code = websocketpp::close::status::normal, const std::string& reason = "");

    //! Disconnected session handler
    void Disconnected();
};

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEBSOCKET_SESSION_H
