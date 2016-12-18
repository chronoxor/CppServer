/*!
    \file tcp_server.h
    \brief TCP server definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TCP_SERVER_H
#define CPPSERVER_ASIO_TCP_SERVER_H

#include "service.h"
#include "tcp_session.h"

#include "system/uuid.h"

#include <map>
#include <memory>
#include <string>
#include <thread>

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class TCPSession;

//! TCP server
/*!
    TCP server is used to connect, disconnect and manage TCP sessions.
    It is implemented based on Asio C++ Library and use a separate
    thread to perform all TCP communications.

    Not thread-safe.
*/
template <class TServer, class TSession>
class TCPServer : public std::enable_shared_from_this<TCPServer<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class TCPSession;

public:
    //! Initialize TCP server with a given Asio service, protocol and port number
    /*!
        \param service - Asio service
        \param protocol - Protocol type
        \param port - Port number
    */
    explicit TCPServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port);
    //! Initialize TCP server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit TCPServer(std::shared_ptr<Service> service, const std::string& address, int port);
    TCPServer(const TCPServer&) = delete;
    TCPServer(TCPServer&&) = default;
    virtual ~TCPServer() { Stop(); }

    TCPServer& operator=(const TCPServer&) = delete;
    TCPServer& operator=(TCPServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service> service() noexcept { return _service; }

    //! Is the service started?
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

    //! Broadcast data into all sessions
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the data was successfully broadcast, 'false' if the server it not started
    */
    bool Broadcast(const void* buffer, size_t size);

    //! Disconnect all sessions
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
    // Server acceptor & socket
    asio::ip::tcp::acceptor _acceptor;
    asio::ip::tcp::socket _socket;
    std::atomic<bool> _started;
    // Server sessions
    std::map<CppCommon::UUID, std::shared_ptr<TSession>> _sessions;
    // Broadcast buffer
    std::mutex _broadcast_lock;
    std::vector<uint8_t> _broadcast_buffer;

    //! Accept new connections
    void Accept();

    //! Register a new session
    std::shared_ptr<TSession> RegisterSession();
    //! Unregister the given session
    void UnregisterSession(const CppCommon::UUID& id);
};

/*! \example tcp_chat_server.cpp TCP chat server example */

} // namespace Asio
} // namespace CppServer

#include "tcp_server.inl"

#endif // CPPSERVER_ASIO_TCP_SERVER_H
