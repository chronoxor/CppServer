/*!
    \file server.h
    \brief Nanomsg server definition
    \author Ivan Shynkarenka
    \date 27.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SERVER_H
#define CPPSERVER_NANOMSG_SERVER_H

#include "socket.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg server
/*!
    Nanomsg server is used to receive messages from Nanomsg clients and
    send responses back to clients.

    Thread-safe.
*/
class Server
{
public:
    //! Initialize server with a given domain and protocol and bind it to the given endpoint
    /*!
        \param domain - Domain
        \param protocol - Protocol
        \param address - Endpoint address
    */
    explicit Server(Domain domain, Protocol protocol, const std::string& address);
    Server(const Server&) = delete;
    Server(Server&&) = default;
    virtual ~Server() = default;

    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = default;

    //! Get the Nanomsg socket
    Socket& socket() noexcept { return _socket; }

    //! Is the server started?
    bool IsStarted() const noexcept { return _socket.IsOpened() && _socket.IsConnected(); }

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
    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

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
    // Nanomsg server address
    std::string _address;
    // Nanomsg socket
    Socket _socket;
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SERVER_H
