/*!
    \file websocket_ssl_client.h
    \brief WebSocket SSL client definition
    \author Ivan Shynkarenka
    \date 11.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEBSOCKET_SSL_CLIENT_H
#define CPPSERVER_ASIO_WEBSOCKET_SSL_CLIENT_H

#include "service.h"
#include "websocket.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

//! WebSocket SSL client
/*!
    WebSocket SSL client is used to read/write data from/into the connected WebSocket SSL server.

    Thread-safe.
*/
class WebSocketSSLClient : public std::enable_shared_from_this<WebSocketSSLClient>
{
public:
    //! Initialize WebSocket client with a given Asio service, SSL context and server URI address
    /*!
        \param service - Asio service
        \param context - SSL context
        \param uri - WebSocket URI address
    */
    explicit WebSocketSSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& uri);
    WebSocketSSLClient(const WebSocketSSLClient&) = delete;
    WebSocketSSLClient(WebSocketSSLClient&&) = default;
    virtual ~WebSocketSSLClient() = default;

    WebSocketSSLClient& operator=(const WebSocketSSLClient&) = delete;
    WebSocketSSLClient& operator=(WebSocketSSLClient&&) = default;

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the client SSL context
    std::shared_ptr<asio::ssl::context>& context() noexcept;
    //! Get the WebSocket URI address
    const std::string& uri() const noexcept { return _uri; }
    //! Get the WebSocket client core
    WebSocketSSLClientCore& core() noexcept { return _core; }

    //! Get the number messages sent by this client
    uint64_t messages_sent() const noexcept { return _messages_sent; }
    //! Get the number messages received by this client
    uint64_t messages_received() const noexcept { return _messages_received; }
    //! Get the number of bytes sent by this client
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by this client
    uint64_t bytes_received() const noexcept { return _bytes_received; }

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
    bool Disconnect(websocketpp::close::status::value code = websocketpp::close::status::normal, const std::string& reason = "") { return Disconnect(false, code, reason); }
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    bool Reconnect();

    //! Send data to the server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \param opcode - Data opcode (default is websocketpp::frame::opcode::binary)
        \return Count of sent bytes
    */
    size_t Send(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::binary);
    //! Send a text string to the server
    /*!
        \param text - Text string to send
        \param opcode - Data opcode (default is websocketpp::frame::opcode::text)
        \return Count of sent bytes
    */
    size_t Send(const std::string& text, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::text);
    //! Send a message to the server
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t Send(WebSocketSSLMessage message);

protected:
    //! Handle client connected notification
    virtual void onConnected() {}
    //! Handle client disconnected notification
    virtual void onDisconnected() {}

    //! Handle message received notification
    /*!
        \param message - Received message
    */
    virtual void onReceived(WebSocketSSLMessage message) {}

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
    // Server SSL context, URI address, client core and client connection
    std::shared_ptr<asio::ssl::context> _context;
    std::string _uri;
    WebSocketSSLClientCore _core;
    WebSocketSSLClientCore::connection_ptr _connection;
    std::atomic<bool> _initialized;
    std::atomic<bool> _connected;
    // Client statistic
    uint64_t _messages_sent;
    uint64_t _messages_received;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;

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

/*! \example websocket_ssl_chat_client.cpp WebSocket SSL chat client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEBSOCKET_SSL_CLIENT_H
