/*!
    \file websocket_server.h
    \brief WebSocket server definition
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEBSOCKET_SERVER_H
#define CPPSERVER_ASIO_WEBSOCKET_SERVER_H

#include "websocket_session.h"

#include <map>
#include <mutex>
#include <tuple>
#include <vector>

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class WebSocketSession;

//! WebSocket server
/*!
    WebSocket server is used to connect, disconnect and manage WebSocket sessions.

    Thread-safe.
*/
template <class TServer, class TSession>
class WebSocketServer : public std::enable_shared_from_this<WebSocketServer<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class WebSocketSession;

public:
    //! Initialize WebSocket server with a given Asio service, protocol and port number
    /*!
        \param service - Asio service
        \param protocol - Protocol type
        \param port - Port number
    */
    explicit WebSocketServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port);
    //! Initialize WebSocket server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit WebSocketServer(std::shared_ptr<Service> service, const std::string& address, int port);
    //! Initialize WebSocket server with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server endpoint
    */
    explicit WebSocketServer(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint);
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer(WebSocketServer&&) = default;
    virtual ~WebSocketServer() = default;

    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer& operator=(WebSocketServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the server endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the WebSocket server core
    WebSocketServerCore& core() noexcept { return _core; }

    //! Get the number of bytes sent by this server
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by this server
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Connected sessions count
    size_t sessions() const noexcept { return _sessions.size(); }

    //! Is the server started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start();
    //! Stop the server
    /*!
        \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
    */
    bool Stop();
    //! Restart the server
    /*!
        \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
    */
    bool Restart();

    //! Multicast data to all connected sessions
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \param opcode - Data opcode (default is websocketpp::frame::opcode::binary)
        \return 'true' if the data was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::binary);
    //! Multicast a text string to all connected sessions
    /*!
        \param text - Text string to send
        \param opcode - Data opcode (default is websocketpp::frame::opcode::text)
        \return 'true' if the text string was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(const std::string& text, websocketpp::frame::opcode::value opcode = websocketpp::frame::opcode::text);
    //! Multicast a message to all connected sessions
    /*!
        \param message - Message to send
        \return 'true' if the message was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(WebSocketMessage message);

    //! Disconnect all connected sessions
    /*!
        \return 'true' if all sessions were successfully disconnected, 'false' if the server it not started
    */
    bool DisconnectAll();

protected:
    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle new session connected notification
    /*!
        \param session - Connected session
    */
    virtual void onConnected(std::shared_ptr<TSession> session) {}
    //! Handle session disconnected notification
    /*!
        \param session - Disconnected session
    */
    virtual void onDisconnected(std::shared_ptr<TSession> session) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Asio service
    std::shared_ptr<Service> _service;
    // Server endpoint & core
    asio::ip::tcp::endpoint _endpoint;
    WebSocketServerCore _core;
    std::atomic<bool> _initialized;
    std::atomic<bool> _started;
    // Server statistic
    size_t _bytes_sent;
    size_t _bytes_received;
    // Server sessions
    std::map<websocketpp::connection_hdl, std::shared_ptr<TSession>, std::owner_less<websocketpp::connection_hdl>> _connections;
    std::map<CppCommon::UUID, std::shared_ptr<TSession>> _sessions;
    // Multicast buffer
    std::mutex _multicast_lock;
    std::vector<std::tuple<std::vector<uint8_t>, websocketpp::frame::opcode::value>> _multicast_buffer;
    std::vector<std::tuple<std::string, websocketpp::frame::opcode::value>> _multicast_text;
    std::vector<WebSocketMessage> _multicast_messages;

    //! Initialize Asio
    void InitAsio();

    //! Register a new session
    /*
        \param connection - WebSocket connection
    */
    std::shared_ptr<TSession> RegisterSession(websocketpp::connection_hdl connection);
    //! Unregister the given session
    /*!
        \param connection - Session connection
    */
    void UnregisterSession(websocketpp::connection_hdl connection);
    //! Unregister the given session
    /*!
        \param id - Session Id
    */
    void UnregisterSession(const CppCommon::UUID& id);

    //! Multicast all
    void MulticastAll();

    //! Clear multicast buffer
    void ClearBuffers();
};

/*! \example websocket_chat_server.cpp WebSocket chat server example */

} // namespace Asio
} // namespace CppServer

#include "websocket_session.inl"
#include "websocket_server.inl"

#endif // CPPSERVER_ASIO_WEBSOCKET_SERVER_H
