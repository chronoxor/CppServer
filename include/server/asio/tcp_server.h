/*!
    \file tcp_server.h
    \brief TCP server definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TCP_SERVER_H
#define CPPSERVER_ASIO_TCP_SERVER_H

#include "tcp_session.h"

#include "system/uuid.h"

#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace CppServer {
namespace Asio {

//! TCP server
/*!
    TCP server is used to connect, disconnect and manage TCP sessions.

    Thread-safe.
*/
class TCPServer : public std::enable_shared_from_this<TCPServer>
{
    friend class TCPSession;

public:
    //! Initialize TCP server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
        \param protocol - Internet protocol type (default is IPv4)
    */
    TCPServer(const std::shared_ptr<Service>& service, int port, InternetProtocol protocol = InternetProtocol::IPv4);
    //! Initialize TCP server with a given Asio service, server address and port number
    /*!
        \param service - Asio service
        \param address - Server address
        \param port - Port number
    */
    TCPServer(const std::shared_ptr<Service>& service, const std::string& address, int port);
    //! Initialize TCP server with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server TCP endpoint
    */
    TCPServer(const std::shared_ptr<Service>& service, const asio::ip::tcp::endpoint& endpoint);
    TCPServer(const TCPServer&) = delete;
    TCPServer(TCPServer&&) = delete;
    virtual ~TCPServer() = default;

    TCPServer& operator=(const TCPServer&) = delete;
    TCPServer& operator=(TCPServer&&) = delete;

    //! Get the server Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }
    //! Get the server endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the server acceptor
    asio::ip::tcp::acceptor& acceptor() noexcept { return _acceptor; }

    //! Get the server address
    const std::string& address() const noexcept { return _address; }
    //! Get the server port number
    int port() const noexcept { return _port; }

    //! Get the number of sessions connected to the server
    uint64_t connected_sessions() const noexcept { return _sessions.size(); }
    //! Get the number of bytes pending sent by the server
    uint64_t bytes_pending() const noexcept { return _bytes_pending; }
    //! Get the number of bytes sent by the server
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the server
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Get the option: keep alive
    bool option_keep_alive() const noexcept { return _option_keep_alive; }
    //! Get the option: no delay
    bool option_no_delay() const noexcept { return _option_no_delay; }
    //! Get the option: reuse address
    bool option_reuse_address() const noexcept { return _option_reuse_address; }
    //! Get the option: reuse port
    bool option_reuse_port() const noexcept { return _option_reuse_port; }

    //! Is the server started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    virtual bool Start();
    //! Stop the server
    /*!
        \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
    */
    virtual bool Stop();
    //! Restart the server
    /*!
        \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
    */
    virtual bool Restart();

    //! Multicast data to all connected sessions
    /*!
        \param buffer - Buffer to multicast
        \param size - Buffer size
        \return 'true' if the data was successfully multicast, 'false' if the server is not started
    */
    virtual bool Multicast(const void* buffer, size_t size);
    //! Multicast text to all connected sessions
    /*!
        \param text - Text to multicast
        \return 'true' if the text was successfully multicast, 'false' if the server is not started
    */
    virtual bool Multicast(std::string_view text) { return Multicast(text.data(), text.size()); }

    //! Disconnect all connected sessions
    /*!
        \return 'true' if all sessions were successfully disconnected, 'false' if the server is not started
    */
    virtual bool DisconnectAll();

    //! Find a session with a given Id
    /*!
        \param id - Session Id
        \return Session with a given Id or null if the session it not connected
    */
    std::shared_ptr<TCPSession> FindSession(const CppCommon::UUID& id);

    //! Setup option: keep alive
    /*!
        This option will setup SO_KEEPALIVE if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupKeepAlive(bool enable) noexcept { _option_keep_alive = enable; }
    //! Setup option: no delay
    /*!
        This option will enable/disable Nagle's algorithm for TCP protocol.

        https://en.wikipedia.org/wiki/Nagle%27s_algorithm

        \param enable - Enable/disable option
    */
    void SetupNoDelay(bool enable) noexcept { _option_no_delay = enable; }
    //! Setup option: reuse address
    /*!
        This option will enable/disable SO_REUSEADDR if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupReuseAddress(bool enable) noexcept { _option_reuse_address = enable; }
    //! Setup option: reuse port
    /*!
        This option will enable/disable SO_REUSEPORT if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupReusePort(bool enable) noexcept { _option_reuse_port = enable; }

protected:
    //! Create TCP session factory method
    /*!
        \param server - TCP server
        \return TCP session
    */
    virtual std::shared_ptr<TCPSession> CreateSession(const std::shared_ptr<TCPServer>& server) { return std::make_shared<TCPSession>(server); }

protected:
    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle session connected notification
    /*!
        \param session - Connected session
    */
    virtual void onConnected(std::shared_ptr<TCPSession>& session) {}
    //! Handle session disconnected notification
    /*!
        \param session - Disconnected session
    */
    virtual void onDisconnected(std::shared_ptr<TCPSession>& session) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

protected:
    // Server sessions
    std::shared_mutex _sessions_lock;
    std::map<CppCommon::UUID, std::shared_ptr<TCPSession>> _sessions;

private:
    // Server Id
    CppCommon::UUID _id;
    // Asio service
    std::shared_ptr<Service> _service;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialized handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Server address, scheme & port
    std::string _address;
    int _port;
    // Server endpoint, acceptor & socket
    std::shared_ptr<TCPSession> _session;
    asio::ip::tcp::endpoint _endpoint;
    asio::ip::tcp::acceptor _acceptor;
    std::atomic<bool> _started;
    HandlerStorage _acceptor_storage;
    // Server statistic
    uint64_t _bytes_pending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Options
    bool _option_keep_alive;
    bool _option_no_delay;
    bool _option_reuse_address;
    bool _option_reuse_port;

    //! Accept new connections
    void Accept();

    //! Register a new session
    void RegisterSession();
    //! Unregister the given session
    /*!
        \param id - Session Id
    */
    void UnregisterSession(const CppCommon::UUID& id);

    //! Clear multicast buffer
    void ClearBuffers();

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example tcp_chat_server.cpp TCP chat server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_TCP_SERVER_H
