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
class WebSocketServer
{
    template <class TSomeServer, class TSomeSession>
    friend class WebSocketSession;

public:
    //! Initialize WebSocket server with a given Asio service and port number
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
    //! Initialize WebSocket server with a given endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server endpoint
    */
    explicit WebSocketServer(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint);
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer(WebSocketServer&&) = default;
    virtual ~WebSocketServer() { Stop(); }

    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer& operator=(WebSocketServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the WebSocket server
    websocket& server() noexcept { return _server; }
    //! Get the server endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }

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
        \return 'true' if the data was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(const void* buffer, size_t size);

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
    // Server endpoint & socket
    asio::ip::tcp::endpoint _endpoint;
    websocket _server;
    std::atomic<bool> _started;
    // Server sessions
    std::map<CppCommon::UUID, std::shared_ptr<TSession>> _sessions;
    // Multicast buffer
    std::mutex _multicast_lock;
    std::vector<uint8_t> _multicast_buffer;

    //! Register a new session
    std::shared_ptr<TSession> RegisterSession();
    //! Unregister the given session
    void UnregisterSession(const CppCommon::UUID& id);

    //! Clear multicast buffer
    void ClearBuffers();
};

/*! \example websocket_echo_server.cpp WebSocket echo server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEBSOCKET_SERVER_H
