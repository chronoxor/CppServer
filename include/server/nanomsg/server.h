/*!
    \file server.h
    \brief Nanomsg server definition
    \author Ivan Shynkarenka
    \date 27.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SERVER_H
#define CPPSERVER_NANOMSG_SERVER_H

#include "socket.h"

#include "threads/thread.h"

#include <atomic>
#include <thread>

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
    //! Initialize server with a given domain, protocol and endpoint address
    /*!
        \param domain - Domain
        \param protocol - Protocol
        \param address - Endpoint address
        \param threading - Run the server in a separate thread (default is true)
    */
    explicit Server(Domain domain, Protocol protocol, const std::string& address, bool threading = true);
    Server(const Server&) = delete;
    Server(Server&&) = default;
    virtual ~Server();

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

    //! Send data to the client
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t Send(const void* buffer, size_t size);
    //! Send a text string to the client
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t Send(const std::string& text) { return Send(text.data(), text.size()); }
    //! Send a message to the client
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t Send(const Message& message) { return Send(message.buffer(), message.size()); }

    //! Try to send data to the client in non-blocking mode
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t TrySend(const void* buffer, size_t size);
    //! Try to send a text string to the client in non-blocking mode
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t TrySend(const std::string& text) { return TrySend(text.data(), text.size()); }
    //! Try to send a message to the client in non-blocking mode
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t TrySend(const Message& message) { return TrySend(message.buffer(), message.size()); }

    //! Receive a message from the client
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    size_t Receive(Message& message);

    //! Try to receive a message from the client in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes
    */
    size_t TryReceive(Message& message);

protected:
    //! Initialize thread handler
    /*!
         This handler can be used to initialize priority or affinity of the server thread.
    */
    virtual void onThreadInitialize() {}
    //! Cleanup thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the server thread.
    */
    virtual void onThreadCleanup() {}

    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle server idle notification
    virtual void onIdle() { CppCommon::Thread::Yield(); }

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
    std::atomic<bool> _connected;
    // Nanomsg server thread
    bool _threading;
    std::thread _thread;
    std::atomic<bool> _joining;
    std::atomic<bool> _receiving;

    //! Server loop
    void ServerLoop();
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SERVER_H
