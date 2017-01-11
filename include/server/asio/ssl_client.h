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
    //! Initialize SSL client with a server IP address and port number
    /*!
        \param service - Asio service
        \param context - SSL context
        \param address - Server IP address
        \param port - Server port number
    */
    explicit SSLClient(std::shared_ptr<Service> service, asio::ssl::context& context, const std::string& address, int port);
    //! Initialize SSL client with a given SSL endpoint
    /*!
        \param service - Asio service
        \param context - SSL context
        \param endpoint - Server SSL endpoint
    */
    explicit SSLClient(std::shared_ptr<Service> service, asio::ssl::context& context, const asio::ip::tcp::endpoint& endpoint);
    SSLClient(const SSLClient&) = delete;
    SSLClient(SSLClient&& client);
    virtual ~SSLClient();

    SSLClient& operator=(const SSLClient&) = delete;
    SSLClient& operator=(SSLClient&& client);

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept;
    //! Get the client SSL context
    asio::ssl::context& context() noexcept;
    //! Get the client endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept;
    //! Get the client SSL stream
    asio::ssl::stream<asio::ip::tcp::socket>& stream() noexcept;
    //! Get the client socket
    asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& socket() noexcept;

    //! Is the client connected?
    bool IsConnected() const noexcept;
    //! Is the session handshaked?
    bool IsHandshaked() const noexcept;

    //! Connect the client
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    bool Connect();
    //! Disconnect the client
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect() { return Disconnect(false); }
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    bool Reconnect();

    //! Send data to the server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of pending bytes in the send buffer
    */
    size_t Send(const void* buffer, size_t size);

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

        Default behavior is to handle all bytes from the received buffer.
        If you want to wait for some more bytes from the server return the
        size of the buffer you want to keep until another chunk is received.

        \param buffer - Received buffer
        \param size - Received buffer size
        \return Count of handled bytes
    */
    virtual size_t onReceived(const void* buffer, size_t size) { return size; }
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
