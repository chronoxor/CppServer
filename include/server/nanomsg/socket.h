/*!
    \file socket.h
    \brief Nanomsg socket definition
    \author Ivan Shynkarenka
    \date 26.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SOCKET_H
#define CPPSERVER_NANOMSG_SOCKET_H

#include "message.h"

#include <string>
#include <tuple>

namespace CppServer {
namespace Nanomsg {

//! Nanomsg socket
/*!
    Nanomsg socket is used as a base communication primitive that wraps all
    necessary Nanomsg library API.

    Thread-safe.

    http://nanomsg.org
*/
class Socket
{
public:
    //! Initialize and open socket with a given domain and protocol
    /*!
        \param domain - Domain
        \param protocol - Protocol
    */
    explicit Socket(Domain domain, Protocol protocol);
    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    ~Socket();

    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = default;

    //! Get the socket domain
    Domain domain() const noexcept { return _domain; }
    //! Get the socket protocol
    Protocol protocol() const noexcept { return _protocol; }
    //! Get the socket handler
    int socket() const noexcept { return _socket; }
    //! Get the socket endpoint
    int endpoint() const noexcept { return _endpoint; }
    //! Get the socket address
    const std::string& address() const noexcept { return _address; }

    //! Get the number of connections successfully established that were initiated from this socket
    uint64_t established_connections() const noexcept;
    //! Get the number of connections successfully established that were accepted by this socket
    uint64_t accepted_connections() const noexcept;
    //! Get the number of established connections that were dropped by this socket
    uint64_t dropped_connections() const noexcept;
    //! Get the number of established connections that were closed by this socket, typically due to protocol errors
    uint64_t broken_connections() const noexcept;
    //! Get the number of errors encountered by this socket trying to connect to a remote peer
    uint64_t connect_errors() const noexcept;
    //! Get the number of errors encountered by this socket trying to bind to a local address
    uint64_t bind_errors() const noexcept;
    //! Get the number of errors encountered by this socket trying to accept a a connection from a remote peer
    uint64_t accept_errors() const noexcept;
    //! Get the number of connections currently estabalished to this socket
    uint64_t current_connections() const noexcept;
    //! Get the number messages sent by this socket
    uint64_t messages_sent() const noexcept;
    //! Get the number messages received by this socket
    uint64_t messages_received() const noexcept;
    //! Get the number of bytes sent by this socket
    uint64_t bytes_sent() const noexcept;
    //! Get the number of bytes received by this socket
    uint64_t bytes_received() const noexcept;

    //! Is socket opened?
    bool IsOpened() const noexcept { return (_socket >= 0); }
    //! Is socket connected?
    bool IsConnected() const noexcept { return (_endpoint >= 0); }

    //! Open the socket
    /*!
        \return 'true' if the socket was successfully opened, 'false' if the socket was already opened
    */
    bool Open();
    //! Close the socket
    /*!
        \return 'true' if the socket was successfully closed, 'false' if the socket was already closed
    */
    bool Close();
    //! Reopen the socket
    /*!
        \return 'true' if the socket was successfully reopened, 'false' if the socket was not reopened
    */
    bool Reopen();

    //! Set the socket option
    /*!
        \param level - Protocol level
        \param option - Socket option
        \param value - Option value pointer
        \param size - Option value size
        \return 'true' if the socket option was successfully set, 'false' if the nanomsg engine terminated
    */
    bool SetSocketOption(int level, int option, const void* value, size_t size);
    //! Get the socket option
    /*!
        \param level - Protocol level
        \param option - Socket option
        \param value - Option value pointer
        \param size - Option value size
        \return 'true' if the socket option was successfully get, 'false' if the nanomsg engine terminated
    */
    bool GetSocketOption(int level, int option, void* value, size_t* size);

    //! Bind the socket to the local endpoint
    /*!
        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the socket was successfully bind, 'false' if the socket was already connected or the nanomsg engine terminated
    */
    bool Bind(const std::string& address);
    //! Connect the socket to the remote endpoint
    /*!
        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the socket was successfully connected, 'false' if the socket was already connected or the nanomsg engine terminated
    */
    bool Connect(const std::string& address);
    //! Link the socket to the remote endpoint
    /*!
        Important: This method works properly only for bus protocol!

        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the socket was successfully linked, 'false' if the current protocol does not support linking or the nanomsg engine terminated
    */
    bool Link(const std::string& address);
    //! Disconnect the socket from the endpoint
    /*!
        Removes an endpoint from the socket. The method call will return immediately, however,
        the library will try to deliver any outstanding outbound messages to the endpoint for
        the time specified by NN_LINGER socket option.

        \return 'true' if the socket was successfully disconnected, 'false' if the socket was already disconnected or the nanomsg engine terminated
    */
    bool Disconnect();

    //! Send data to the socket
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t Send(const void* buffer, size_t size);
    //! Send a text string to the socket
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t Send(const std::string& text) { return Send(text.data(), text.size()); }
    //! Send a message to the socket
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t Send(const Message& message) { return Send(message.buffer(), message.size()); }

    //! Try to send data to the socket in non-blocking mode
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t TrySend(const void* buffer, size_t size);
    //! Try to send a text string to the socket in non-blocking mode
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t TrySend(const std::string& text) { return TrySend(text.data(), text.size()); }
    //! Try to send a message to the socket in non-blocking mode
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t TrySend(const Message& message) { return TrySend(message.buffer(), message.size()); }

    //! Receive a message from the socket in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    size_t Receive(Message& message);

    //! Try to receive a message from the socket in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    size_t TryReceive(Message& message);

    //! Receive a respond to the survey from the socket in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> ReceiveSurvey(Message& message);

    //! Try to receive a respond to the survey from the socket in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> TryReceiveSurvey(Message& message);

    //! Terminate all socket operations
    static void Terminate();

private:
    Domain _domain;
    Protocol _protocol;
    int _socket;
    int _endpoint;
    std::string _address;
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SOCKET_H
