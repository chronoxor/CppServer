/*!
    \file websocket_ssl_server.h
    \brief WebSocket SSL server definition
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEBSOCKET_SSL_SERVER_H
#define CPPSERVER_ASIO_WEBSOCKET_SSL_SERVER_H

#include "websocket_ssl_session.h"

#include <map>
#include <mutex>
#include <tuple>
#include <vector>

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class WebSocketSSLSession;

//! WebSocket SSL server
/*!
    WebSocket SSL server is used to connect, disconnect and manage WebSocket sessions.

    Thread-safe.
*/
template <class TServer, class TSession>
class WebSocketSSLServer : public std::enable_shared_from_this<WebSocketSSLServer<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class WebSocketSSLSession;

public:
    //! Initialize WebSocket server with a given Asio service, SSL context, protocol and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param protocol - Protocol type
        \param port - Port number
    */
    explicit WebSocketSSLServer(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, InternetProtocol protocol, int port);
    //! Initialize WebSocket server with a given Asio service, SSL context, IP address and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param address - IP address
        \param port - Port number
    */
    explicit WebSocketSSLServer(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port);
    //! Initialize WebSocket server with a given Asio service, SSL context and endpoint
    /*!
        \param service - Asio service
        \param context - SSL context
        \param endpoint - Server endpoint
    */
    explicit WebSocketSSLServer(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint);
    WebSocketSSLServer(const WebSocketSSLServer&) = delete;
    WebSocketSSLServer(WebSocketSSLServer&&) = default;
    virtual ~WebSocketSSLServer() = default;

    WebSocketSSLServer& operator=(const WebSocketSSLServer&) = delete;
    WebSocketSSLServer& operator=(WebSocketSSLServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the server SSL context
    std::shared_ptr<asio::ssl::context>& context() noexcept { return _context; }
    //! Get the server endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the WebSocket server core
    WebSocketSSLServerCore& core() noexcept { return _core; }

    //! Get the number of sessions currently connected to this server
    uint64_t current_sessions() const noexcept { return _sessions.size(); }
    //! Get the number messages sent by this server
    uint64_t messages_sent() const noexcept { return _messages_sent; }
    //! Get the number messages received by this server
    uint64_t messages_received() const noexcept { return _messages_received; }
    //! Get the number of bytes sent by this server
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by this server
    uint64_t bytes_received() const noexcept { return _bytes_received; }

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
    bool Multicast(WebSocketSSLMessage message);

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
    // Server SSL context, endpoint & core
    std::shared_ptr<asio::ssl::context> _context;
    asio::ip::tcp::endpoint _endpoint;
    WebSocketSSLServerCore _core;
    std::atomic<bool> _initialized;
    std::atomic<bool> _started;
    // Server statistic
    uint64_t _messages_sent;
    uint64_t _messages_received;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Server sessions
    std::map<websocketpp::connection_hdl, std::shared_ptr<TSession>, std::owner_less<websocketpp::connection_hdl>> _connections;
    std::map<CppCommon::UUID, std::shared_ptr<TSession>> _sessions;
    // Multicast buffer
    std::mutex _multicast_lock;
    std::vector<std::tuple<std::vector<uint8_t>, websocketpp::frame::opcode::value>> _multicast_buffer;
    std::vector<std::tuple<std::string, websocketpp::frame::opcode::value>> _multicast_text;
    std::vector<WebSocketSSLMessage> _multicast_messages;

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

/*! \example websocket_ssl_chat_server.cpp WebSocket SSL chat server example */

} // namespace Asio
} // namespace CppServer

#include "websocket_ssl_session.inl"
#include "websocket_ssl_server.inl"

#endif // CPPSERVER_ASIO_WEBSOCKET_SSL_SERVER_H
