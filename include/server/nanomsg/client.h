/*!
    \file client.h
    \brief Nanomsg client definition
    \author Ivan Shynkarenka
    \date 28.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_CLIENT_H
#define CPPSERVER_NANOMSG_CLIENT_H

#include "socket.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg client
/*!
    Nanomsg client is used to send messages to Nanomsg server and
    receive responses back.

    Thread-safe.
*/
class Client
{
public:
    //! Initialize client with a given domain, protocol and endpoint address
    /*!
        \param domain - Domain
        \param protocol - Protocol
        \param address - Endpoint address
    */
    explicit Client(Domain domain, Protocol protocol, const std::string& address);
    Client(const Client&) = delete;
    Client(Client&&) = default;
    virtual ~Client() = default;

    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = default;

    //! Get the Nanomsg socket
    Socket& socket() noexcept { return _socket; }

    //! Is the client connected?
    bool IsConnected() const noexcept { return _socket.IsOpened() && _socket.IsConnected(); }

    //! Connect the client
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    bool Connect();
    //! Disconnect the client
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect();
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client failed to reconnect
    */
    bool Reconnect();

    //! Send data to the server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    virtual size_t Send(const void* buffer, size_t size);
    //! Send a text string to the server
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    virtual size_t Send(const std::string& text) { return Send(text.data(), text.size()); }
    //! Send a message to the server
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    virtual size_t Send(const Message& message) { return Send(message.buffer(), message.size()); }

    //! Try to send data to the server in non-blocking mode
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    virtual size_t TrySend(const void* buffer, size_t size);
    //! Try to send a text string to the server in non-blocking mode
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    virtual size_t TrySend(const std::string& text) { return TrySend(text.data(), text.size()); }
    //! Try to send a message to the server in non-blocking mode
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    virtual size_t TrySend(const Message& message) { return TrySend(message.buffer(), message.size()); }

    //! Receive a message from the server in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    virtual size_t Receive(Message& message);

    //! Try to receive a message from the server in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    virtual size_t TryReceive(Message& message);

protected:
    //! Handle client connected notification
    virtual void onConnected() {}
    //! Handle client disconnected notification
    virtual void onDisconnected() {}

    //! Handle message received notification
    /*!
        \param message - Received message
    */
    virtual void onReceived(Message& message) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param message - Error message
    */
    virtual void onError(int error, const std::string& message) {}

private:
    // Nanomsg endpoint address
    std::string _address;
    // Nanomsg socket
    Socket _socket;
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_CLIENT_H
