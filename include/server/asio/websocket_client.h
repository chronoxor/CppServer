/*!
    \file websocket_client.h
    \brief WebSocket client definition
    \author Ivan Shynkarenka
    \date 11.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEBSOCKET_CLIENT_H
#define CPPSERVER_ASIO_WEBSOCKET_CLIENT_H

#include "service.h"
#include "websocket.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

//! WebSocket client
/*!
    WebSocket client is used to read/write data from/into the connected WebSocket server.

    Thread-safe.
*/
class WebSocketClient : public std::enable_shared_from_this<WebSocketClient>
{
public:
    //! Initialize WebSocket client with a server IP address and port number
    /*!
        \param service - Asio service
        \param uri - WebSocket URI address
    */
    explicit WebSocketClient(std::shared_ptr<Service> service, const std::string& uri);
    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient(WebSocketClient&&) = default;
    virtual ~WebSocketClient() = default;

    WebSocketClient& operator=(const WebSocketClient&) = delete;
    WebSocketClient& operator=(WebSocketClient&&) = default;

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the WebSocket URI address
    const std::string& uri() const noexcept { return _uri; }
    //! Get the WebSocket client core
    WebSocketClientCore& core() noexcept { return _core; }

    //! Is the client connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Connect the client
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    bool Connect();
    //! Disconnect the client
    /*!
        \param code - Close code to send (default is normal)
        \param reason - Close reason to send (default is "")
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect(websocketpp::close::status::value code = websocketpp::close::status::normal, const std::string& reason = "")
    { return Disconnect(false, code, reason); }
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    bool Reconnect();

    //! Send data to the server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \param opcode - Data opcode (default is binary)
        \return Count of sent bytes
    */
    size_t Send(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::binary);

protected:
    //! Handle client connected notification
    virtual void onConnected() {}
    //! Handle client disconnected notification
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
    // Client Id
    CppCommon::UUID _id;
    // Asio service
    std::shared_ptr<Service> _service;
    // WebSocket server URI address, client core and client connection
    std::string _uri;
    WebSocketClientCore _core;
    WebSocketClientCore::connection_ptr _connection;
    std::atomic<bool> _initialized;
    std::atomic<bool> _connected;

    //! Initialize Asio
    void InitAsio();

    //! Disconnect the client
    /*!
        \param dispatch - Dispatch flag
        \param code - Close code to send (default is normal)
        \param reason - Close reason to send (default is "")
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect(bool dispatch, websocketpp::close::status::value code = websocketpp::close::status::normal, const std::string& reason = "");

    //! Connected session handler
    void Connected();
    //! Disconnected session handler
    void Disconnected();
};

/*! \example websocket_chat_client.cpp WebSocket chat client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEBSOCKET_CLIENT_H
