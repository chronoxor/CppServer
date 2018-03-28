/*!
    \file ssl_client.h
    \brief SSL client definition
    \author Ivan Shynkarenka
    \date 01.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_SSL_CLIENT_H
#define CPPSERVER_ASIO_SSL_CLIENT_H

#include "service.h"

#include "system/uuid.h"

#include <memory>

namespace CppServer {
namespace Asio {

//! SSL client
/*!
    SSL client is used to read/write data from/into the connected SSL server.

    Thread-safe.
*/
class SSLClient : public std::enable_shared_from_this<SSLClient>
{
public:
    //! Initialize SSL client with a given Asio service, server IP address and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param address - Server IP address
        \param port - Server port number
    */
    SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port);
    //! Initialize SSL client with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param context - SSL context
        \param endpoint - Server SSL endpoint
    */
    SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint);
    SSLClient(const SSLClient&) = delete;
    SSLClient(SSLClient&& client);
    virtual ~SSLClient();

    SSLClient& operator=(const SSLClient&) = delete;
    SSLClient& operator=(SSLClient&& client);

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept;
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept;
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept;
    //! Get the client SSL context
    std::shared_ptr<asio::ssl::context>& context() noexcept;
    //! Get the client endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept;
    //! Get the client SSL stream
    asio::ssl::stream<asio::ip::tcp::socket>& stream() noexcept;
    //! Get the client socket
    asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& socket() noexcept;

    //! Get the number of bytes pending sent by the client
    uint64_t bytes_pending() const noexcept;
    //! Get the number of bytes sent by the client
    uint64_t bytes_sent() const noexcept;
    //! Get the number of bytes received by the client
    uint64_t bytes_received() const noexcept;

    //! Get the option: no delay
    bool option_no_delay() const noexcept;

    //! Is the client connected?
    bool IsConnected() const noexcept;
    //! Is the session handshaked?
    bool IsHandshaked() const noexcept;

    //! Connect the client
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool Connect();
    //! Disconnect the client
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool Disconnect() { return Disconnect(false); }
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    virtual bool Reconnect();

    //! Send data to the server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of pending bytes in the send buffer
    */
    virtual size_t Send(const void* buffer, size_t size);
    //! Send a text string to the server
    /*!
        \param text - Text string to send
        \return Count of pending bytes in the send buffer
    */
    virtual size_t Send(const std::string& text) { return Send(text.data(), text.size()); }

    //! Setup option: no delay
    /*!
        This option will enable/disable Nagle's algorithm for TCP protocol.

        https://en.wikipedia.org/wiki/Nagle%27s_algorithm

        \param enable - Enable/disable option
    */
    void SetupNoDelay(bool enable) noexcept;

protected:
    //! Handle client connected notification
    virtual void onConnected() {}
    //! Handle session handshaked notification
    virtual void onHandshaked() {}
    //! Handle client disconnected notification
    virtual void onDisconnected() {}

    //! Handle buffer received notification
    /*!
        Notification is called when another chunk of buffer was received
        from the server.

        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onReceived(const void* buffer, size_t size) {}
    //! Handle buffer sent notification
    /*!
        Notification is called when another chunk of buffer was sent
        to the server.

        This handler could be used to send another buffer to the server
        for instance when the pending size is zero.

        \param sent - Size of sent buffer
        \param pending - Size of pending buffer
    */
    virtual void onSent(size_t sent, size_t pending) {}

    //! Handle empty send buffer notification
    /*!
        Notification is called when the send buffer is empty and ready
        for a new data to send.

        This handler could be used to send another buffer to the server.
    */
    virtual void onEmpty() {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    friend class Impl;
    class Impl;
    std::shared_ptr<Impl> _pimpl;

    //! Disconnect the client
    /*!
        \param dispatch - Dispatch flag
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect(bool dispatch);

    //! Handle client reset notification
    void onReset();
};

/*! \example ssl_chat_client.cpp SSL chat client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_SSL_CLIENT_H
