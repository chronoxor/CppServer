/*!
    \file tcp_session.h
    \brief TCP session definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TCP_SESSION_H
#define CPPSERVER_ASIO_TCP_SESSION_H

#include "service.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

class TCPServer;

//! TCP session
/*!
    TCP session is used to read and write data from the connected TCP client.

    Thread-safe.
*/
class TCPSession : public std::enable_shared_from_this<TCPSession>
{
    friend class TCPServer;

public:
    //! Initialize the session with a given server
    /*!
        \param server - Connected server
    */
    TCPSession(std::shared_ptr<TCPServer> server);
    TCPSession(const TCPSession&) = delete;
    TCPSession(TCPSession&&) = default;
    virtual ~TCPSession() = default;

    TCPSession& operator=(const TCPSession&) = delete;
    TCPSession& operator=(TCPSession&&) = default;

    //! Get the session Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the server
    std::shared_ptr<TCPServer>& server() noexcept { return _server; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }
    //! Get the session socket
    asio::ip::tcp::socket& socket() noexcept { return _socket; }

    //! Get the number of bytes pending sent by the session
    uint64_t bytes_pending() const noexcept { return _bytes_pending; }
    //! Get the number of bytes sent by the session
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the session
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the session connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Disconnect the session
    /*!
        \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
    */
    virtual bool Disconnect() { return Disconnect(false); }

    //! Send data into the session
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the data was successfully sent, 'false' if the session is not connected
    */
    virtual bool Send(const void* buffer, size_t size);
    //! Send a text string into the session
    /*!
        \param text - Text string to send
        \return 'true' if the data was successfully sent, 'false' if the session is not connected
    */
    virtual bool Send(const std::string& text) { return Send(text.data(), text.size()); }

    //! Setup option: receive buffer size
    /*!
        This option will setup SO_RCVBUF if the OS support this feature.

        \param size - Receive buffer size
    */
    void SetupReceiveBufferSize(size_t size);
    //! Setup option: send buffer size
    /*!
        This option will setup SO_SNDBUF if the OS support this feature.

        \param size - Send buffer size
    */
    void SetupSendBufferSize(size_t size);

protected:
    //! Handle session connected notification
    virtual void onConnected() {}
    //! Handle session disconnected notification
    virtual void onDisconnected() {}

    //! Handle buffer received notification
    /*!
        Notification is called when another chunk of buffer was received
        from the client.

        \param buffer - Received buffer
        \param size - Received buffer size
    */
    virtual void onReceived(const void* buffer, size_t size) {}
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

    //! Handle empty send buffer notification
    /*!
        Notification is called when the send buffer is empty and ready
        for a new data to send.

        This handler could be used to send another buffer to the client.
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
    // Session Id
    CppCommon::UUID _id;
    // Server & session
    std::shared_ptr<TCPServer> _server;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialized handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Session socket
    asio::ip::tcp::socket _socket;
    std::atomic<bool> _connected;
    // Session statistic
    uint64_t _bytes_pending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive buffer & cache
    bool _reciving;
    std::vector<uint8_t> _recive_buffer;
    HandlerStorage _recive_storage;
    // Send buffer & cache
    bool _sending;
    std::mutex _send_lock;
    std::vector<uint8_t> _send_buffer_main;
    std::vector<uint8_t> _send_buffer_flush;
    size_t _send_buffer_flush_offset;
    HandlerStorage _send_storage;

    //! Connect the session
    void Connect();
    //! Disconnect the session
    /*!
        \param dispatch - Dispatch flag
        \return 'true' if the session was successfully disconnected, 'false' if the session is already disconnected
    */
    bool Disconnect(bool dispatch);

    //! Try to receive new data
    void TryReceive();
    //! Try to send pending data
    void TrySend();

    //! Clear receive & send buffers
    void ClearBuffers();

    //! Send error notification
    void SendError(std::error_code ec);
};

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_TCP_SESSION_H
