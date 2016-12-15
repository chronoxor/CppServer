/*!
    \file session.h
    \brief TCP session definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_TCP_SESSION_H
#define CPPSERVER_TCP_SESSION_H

#include "system/uuid.h"

#include "../asio.h"

#include <mutex>
#include <vector>

namespace CppServer {

//! TCP session
/*!
    TCP session is used to read and write data from the connected TCP client.

    Not thread-safe.
*/
template <class TServer, class TSession>
class TCPSession
{
public:
    //! Initialize TCP session with a given TCP server and connected socket
    /*!
        \param server - TCP server
        \param socket - Connected socket
    */
    explicit TCPSession(TServer& server, const CppCommon::UUID& uuid, asio::ip::tcp::socket socket);
    TCPSession(const TCPSession&) = delete;
    TCPSession(TCPSession&&) noexcept = default;
    virtual ~TCPSession() {};

    TCPSession& operator=(const TCPSession&) = delete;
    TCPSession& operator=(TCPSession&&) noexcept = default;

    //! Get the session Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Is the session connected?
    bool IsConnected() const noexcept { return _socket.is_open(); };

    //! Disconnect the session
    void Disconnect();

    //! Send data into the session
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of total bytes to send
    */
    size_t Send(const void* buffer, size_t size);

protected:
    //! Handle session connected notification
    virtual void onConnected() {}
    //! Handle session disconnected notification
    virtual void onDisconnected() {}

    //! Handle buffer received notification
    /*!
        Notification is called when another chunk of buffer was received
        from the client.

        Default behavior is to handle all bytes from the received buffer.
        If you want to wait for some more bytes from the client return the
        size of the buffer you want to keep until another chunk is received.

        \param buffer - Received buffer
        \param size - Received buffer size
        \return Count of handled bytes
    */
    virtual size_t onReceived(const void* buffer, size_t size) { return size; }
    //! Handle buffer sent notification
    /*!
        Notification is called when another chunk of buffer was sent
        to the client.

        This handler could be used to send another buffer to the client
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
    TServer& _server;
    CppCommon::UUID _id;
    asio::ip::tcp::socket _socket;
    std::vector<uint8_t> _recive_buffer;
    std::vector<uint8_t> _send_buffer;
    std::mutex _send_lock;

    //! Try to receive data
    void TryReceive();
    //! Try to send data
    void TrySend();
};

} // namespace CppServer

#include "session.inl"

#endif // CPPSERVER_TCP_SESSION_H
