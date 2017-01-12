/*!
    \file ssl_server.h
    \brief SSL server definition
    \author Ivan Shynkarenka
    \date 30.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_SSL_SERVER_H
#define CPPSERVER_ASIO_SSL_SERVER_H

#include "ssl_session.h"

#include <map>
#include <mutex>
#include <vector>

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
class SSLSession;

//! SSL server
/*!
    SSL server is used to connect, disconnect and manage SSL sessions.

    Thread-safe.
*/
template <class TServer, class TSession>
class SSLServer : public std::enable_shared_from_this<SSLServer<TServer, TSession>>
{
    template <class TSomeServer, class TSomeSession>
    friend class SSLSession;

public:
    //! Initialize SSL server with a given Asio service, protocol and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param protocol - Protocol type
        \param port - Port number
    */
    explicit SSLServer(std::shared_ptr<Service> service, asio::ssl::context& context, InternetProtocol protocol, int port);
    //! Initialize SSL server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param address - IP address
        \param port - Port number
    */
    explicit SSLServer(std::shared_ptr<Service> service, asio::ssl::context& context, const std::string& address, int port);
    //! Initialize SSL server with a given SSL endpoint
    /*!
        \param service - Asio service
        \param context - SSL context
        \param endpoint - Server SSL endpoint
    */
    explicit SSLServer(std::shared_ptr<Service> service, asio::ssl::context& context, const asio::ip::tcp::endpoint& endpoint);
    SSLServer(const SSLServer&) = delete;
    SSLServer(SSLServer&&) = default;
    virtual ~SSLServer() = default;

    SSLServer& operator=(const SSLServer&) = delete;
    SSLServer& operator=(SSLServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the server SSL context
    asio::ssl::context& context() noexcept { return _context; }
    //! Get the server endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the server acceptor
    asio::ip::tcp::acceptor& acceptor() noexcept { return _acceptor; }

    //! Total bytes received
    size_t total_received() const noexcept { return _total_received; }
    //! Total bytes sent
    size_t total_sent() const noexcept { return _total_sent; }

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
        \param buffer - Buffer to multicast
        \param size - Buffer size
        \return 'true' if the data was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(const void* buffer, size_t size);
    //! Multicast a text string to all connected sessions
    /*!
        \param text - Text string to multicast
        \return 'true' if the text string was successfully multicast, 'false' if the server it not started
    */
    bool Multicast(const std::string& text) { return Multicast(text.data(), text.size()); }

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
    // Server SSL context, endpoint, acceptor and socket
    asio::ssl::context& _context;
    asio::ip::tcp::endpoint _endpoint;
    asio::ip::tcp::acceptor _acceptor;
    asio::ip::tcp::socket _socket;
    std::atomic<bool> _started;
    // Server statistic
    size_t _total_received;
    size_t _total_sent;
    // Server sessions
    std::map<CppCommon::UUID, std::shared_ptr<TSession>> _sessions;
    // Multicast buffer
    std::mutex _multicast_lock;
    std::vector<uint8_t> _multicast_buffer;

    //! Accept new connections
    void Accept();

    //! Register a new session
    std::shared_ptr<TSession> RegisterSession();
    //! Unregister the given session
    /*!
        \param id - Session Id
    */
    void UnregisterSession(const CppCommon::UUID& id);

    //! Clear multicast buffer
    void ClearBuffers();
};

/*! \example ssl_chat_server.cpp SSL chat server example */

} // namespace Asio
} // namespace CppServer

#include "ssl_server.inl"

#endif // CPPSERVER_ASIO_SSL_SERVER_H
