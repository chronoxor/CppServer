/*!
    \file tcp_client.h
    \brief TCP client definition
    \author Ivan Shynkarenka
    \date 15.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TCP_CLIENT_H
#define CPPSERVER_ASIO_TCP_CLIENT_H

#include "service.h"

#include "system/uuid.h"

#include <mutex>
#include <vector>

namespace CppServer {
namespace Asio {

//! TCP client
/*!
    TCP client is used to read/write data from/into the connected TCP server.

    Thread-safe.
*/
class TCPClient : public std::enable_shared_from_this<TCPClient>
{
public:
    //! Initialize TCP client with a given Asio service, server IP address and port number
    /*!
        \param service - Asio service
        \param address - Server IP address
        \param port - Server port number
    */
    TCPClient(std::shared_ptr<Service> service, const std::string& address, int port);
    //! Initialize TCP client with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server TCP endpoint
    */
    TCPClient(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint);
    TCPClient(const TCPClient&) = delete;
    TCPClient(TCPClient&&) = default;
    virtual ~TCPClient() = default;

    TCPClient& operator=(const TCPClient&) = delete;
    TCPClient& operator=(TCPClient&&) = default;

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }
    //! Get the client endpoint
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the client socket
    asio::ip::tcp::socket& socket() noexcept { return _socket; }

    //! Get the number of bytes pending sent by the client
    uint64_t bytes_pending() const noexcept { return _bytes_pending + _bytes_sending; }
    //! Get the number of bytes sent by the client
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the client
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Get the option: keep alive
    bool option_keep_alive() const noexcept { return _option_keep_alive; }
    //! Get the option: no delay
    bool option_no_delay() const noexcept { return _option_no_delay; }
    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the client connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Connect the client (synchronous)
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool Connect();
    //! Disconnect the client (synchronous)
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool Disconnect();
    //! Reconnect the client (synchronous)
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    virtual bool Reconnect();

    //! Connect the client (asynchronous)
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool ConnectAsync();
    //! Disconnect the client (asynchronous)
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool DisconnectAsync() { return DisconnectAsync(false); }
    //! Reconnect the client (asynchronous)
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    virtual bool ReconnectAsync();

    //! Send data to the server (synchronous)
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Size of sent data
    */
    virtual size_t Send(const void* buffer, size_t size);
    //! Send text to the server (synchronous)
    /*!
        \param text - Text to send
        \return Size of sent text
    */
    virtual size_t Send(const std::string_view& text) { return Send(text.data(), text.size()); }

    //! Send data to the server (asynchronous)
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the data was successfully sent, 'false' if the client is not connected
    */
    virtual bool SendAsync(const void* buffer, size_t size);
    //! Send text to the server (asynchronous)
    /*!
        \param text - Text to send
        \return 'true' if the text was successfully sent, 'false' if the client is not connected
    */
    virtual bool SendAsync(const std::string_view& text) { return SendAsync(text.data(), text.size()); }

    //! Receive data from the server (synchronous)
    /*!
        \param buffer - Buffer to receive
        \param size - Buffer size to receive
        \return Size of received data
    */
    virtual size_t Receive(void* buffer, size_t size);
    //! Receive text from the server (synchronous)
    /*!
        \param size - Text size to receive
        \return Received text
    */
    virtual std::string Receive(size_t size);

    //! Receive data from the server (asynchronous)
    virtual void ReceiveAsync();

    //! Setup option: keep alive
    /*!
        This option will setup SO_KEEPALIVE if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupKeepAlive(bool enable) noexcept { _option_keep_alive = enable; }
    //! Setup option: no delay
    /*!
        This option will enable/disable Nagle's algorithm for TCP protocol.

        https://en.wikipedia.org/wiki/Nagle%27s_algorithm

        \param enable - Enable/disable option
    */
    void SetupNoDelay(bool enable) noexcept { _option_no_delay = enable; }
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
    //! Handle client connected notification
    virtual void onConnected() {}
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
    // Client Id
    CppCommon::UUID _id;
    // Asio service
    std::shared_ptr<Service> _service;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialized handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Server endpoint & client socket
    asio::ip::tcp::endpoint _endpoint;
    asio::ip::tcp::socket _socket;
    std::atomic<bool> _connecting;
    std::atomic<bool> _connected;
    // Client statistic
    uint64_t _bytes_pending;
    uint64_t _bytes_sending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive buffer
    bool _receiving;
    std::vector<uint8_t> _receive_buffer;
    HandlerStorage _receive_storage;
    // Send buffer
    bool _sending;
    std::mutex _send_lock;
    std::vector<uint8_t> _send_buffer_main;
    std::vector<uint8_t> _send_buffer_flush;
    size_t _send_buffer_flush_offset;
    HandlerStorage _send_storage;
    // Options
    bool _option_keep_alive;
    bool _option_no_delay;

    //! Disconnect the client (asynchronous)
    /*!
        \param dispatch - Dispatch flag
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool DisconnectAsync(bool dispatch);

    //! Try to receive new data
    void TryReceive();
    //! Try to send pending data
    void TrySend();

    //! Clear send/receive buffers
    void ClearBuffers();

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example tcp_chat_client.cpp TCP chat client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_TCP_CLIENT_H
