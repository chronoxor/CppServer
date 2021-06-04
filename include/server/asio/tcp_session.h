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
    explicit TCPSession(const std::shared_ptr<TCPServer>& server);
    TCPSession(const TCPSession&) = delete;
    TCPSession(TCPSession&&) = delete;
    virtual ~TCPSession() = default;

    TCPSession& operator=(const TCPSession&) = delete;
    TCPSession& operator=(TCPSession&&) = delete;

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
    uint64_t bytes_pending() const noexcept { return _bytes_pending + _bytes_sending; }
    //! Get the number of bytes sent by the session
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the session
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Get the option: receive buffer limit
    size_t option_receive_buffer_limit() const noexcept { return _receive_buffer_limit; }
    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer limit
    size_t option_send_buffer_limit() const noexcept { return _send_buffer_limit; }
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the session connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Disconnect the session
    /*!
        \return 'true' if the section was successfully disconnected, 'false' if the section is already disconnected
    */
    virtual bool Disconnect() { return Disconnect(false); }

    //! Send data to the client (synchronous)
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Size of sent data
    */
    virtual size_t Send(const void* buffer, size_t size);
    //! Send text to the client (synchronous)
    /*!
        \param text - Text to send
        \return Size of sent text
    */
    virtual size_t Send(std::string_view text) { return Send(text.data(), text.size()); }

    //! Send data to the client with timeout (synchronous)
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \param timeout - Timeout
        \return Size of sent data
    */
    virtual size_t Send(const void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Send text to the client with timeout (synchronous)
    /*!
        \param text - Text to send
        \param timeout - Timeout
        \return Size of sent text
    */
    virtual size_t Send(std::string_view text, const CppCommon::Timespan& timeout) { return Send(text.data(), text.size(), timeout); }

    //! Send data to the client (asynchronous)
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the data was successfully sent, 'false' if the session is not connected
    */
    virtual bool SendAsync(const void* buffer, size_t size);
    //! Send text to the client (asynchronous)
    /*!
        \param text - Text to send
        \return 'true' if the text was successfully sent, 'false' if the session is not connected
    */
    virtual bool SendAsync(std::string_view text) { return SendAsync(text.data(), text.size()); }

    //! Receive data from the client (synchronous)
    /*!
        \param buffer - Buffer to receive
        \param size - Buffer size to receive
        \return Size of received data
    */
    virtual size_t Receive(void* buffer, size_t size);
    //! Receive text from the client (synchronous)
    /*!
        \param size - Text size to receive
        \return Received text
    */
    virtual std::string Receive(size_t size);

    //! Receive data from the client with timeout (synchronous)
    /*!
        \param buffer - Buffer to receive
        \param size - Buffer size to receive
        \param timeout - Timeout
        \return Size of received data
    */
    virtual size_t Receive(void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Receive text from the client with timeout (synchronous)
    /*!
        \param size - Text size to receive
        \param timeout - Timeout
        \return Received text
    */
    virtual std::string Receive(size_t size, const CppCommon::Timespan& timeout);

    //! Receive data from the client (asynchronous)
    virtual void ReceiveAsync();

    //! Setup option: receive buffer limit
    /*!
        The session will be disconnected if the receive buffer limit is met.
        Default is unlimited.

        \param limit - Receive buffer limit
    */
    void SetupReceiveBufferLimit(size_t limit) noexcept { _receive_buffer_limit = limit; }
    //! Setup option: receive buffer size
    /*!
        This option will setup SO_RCVBUF if the OS support this feature.

        \param size - Receive buffer size
    */
    void SetupReceiveBufferSize(size_t size);
    //! Setup option: send buffer limit
    /*!
        The session will be disconnected if the send buffer limit is met.
        Default is unlimited.

        \param limit - Send buffer limit
    */
    void SetupSendBufferLimit(size_t limit) noexcept { _send_buffer_limit = limit; }
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
    uint64_t _bytes_sending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive buffer
    bool _receiving;
    size_t _receive_buffer_limit{0};
    std::vector<uint8_t> _receive_buffer;
    HandlerStorage _receive_storage;
    // Send buffer
    bool _sending;
    std::mutex _send_lock;
    size_t _send_buffer_limit{0};
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

    //! Clear send/receive buffers
    void ClearBuffers();
    //! Reset server
    void ResetServer();

    //! Send error notification
    void SendError(std::error_code ec);
};

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_TCP_SESSION_H
