/*!
    \file udp_server.h
    \brief UDP server definition
    \author Ivan Shynkarenka
    \date 22.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_UDP_SERVER_H
#define CPPSERVER_ASIO_UDP_SERVER_H

#include "service.h"

#include "system/uuid.h"

namespace CppServer {
namespace Asio {

//! UDP server
/*!
    UDP server is used to send or multicast datagrams to UDP endpoints.

    Thread-safe.
*/
class UDPServer : public std::enable_shared_from_this<UDPServer>
{
public:
    //! Initialize UDP server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
        \param protocol - Internet protocol type (default is IPv4)
    */
    UDPServer(const std::shared_ptr<Service>& service, int port, InternetProtocol protocol = InternetProtocol::IPv4);
    //! Initialize UDP server with a given Asio service, server address and port number
    /*!
        \param service - Asio service
        \param address - Server address
        \param port - Port number
    */
    UDPServer(const std::shared_ptr<Service>& service, const std::string& address, int port);
    //! Initialize UDP server with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server UDP endpoint
    */
    UDPServer(const std::shared_ptr<Service>& service, const asio::ip::udp::endpoint& endpoint);
    UDPServer(const UDPServer&) = delete;
    UDPServer(UDPServer&&) = delete;
    virtual ~UDPServer() = default;

    UDPServer& operator=(const UDPServer&) = delete;
    UDPServer& operator=(UDPServer&&) = delete;

    //! Get the server Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }
    //! Get the server endpoint
    asio::ip::udp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the server multicast endpoint
    asio::ip::udp::endpoint& multicast_endpoint() noexcept { return _multicast_endpoint; }

    //! Get the server address
    const std::string& address() const noexcept { return _address; }
    //! Get the server port number
    int port() const noexcept { return _port; }

    //! Get the number of bytes pending sent by the server
    uint64_t bytes_pending() const noexcept { return _bytes_sending; }
    //! Get the number of bytes sent by the server
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the server
    uint64_t bytes_received() const noexcept { return _bytes_received; }
    //! Get the number datagrams sent by the server
    uint64_t datagrams_sent() const noexcept { return _datagrams_sent; }
    //! Get the number datagrams received by the server
    uint64_t datagrams_received() const noexcept { return _datagrams_received; }

    //! Get the option: reuse address
    bool option_reuse_address() const noexcept { return _option_reuse_address; }
    //! Get the option: reuse port
    bool option_reuse_port() const noexcept { return _option_reuse_port; }
    //! Get the option: receive buffer limit
    size_t option_receive_buffer_limit() const noexcept { return _receive_buffer_limit; }
    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer limit
    size_t option_send_buffer_limit() const noexcept { return _send_buffer_limit; }
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the server started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    virtual bool Start();
    //! Start the server with a given multicast address and port number
    /*!
        \param multicast_address - Multicast address
        \param multicast_port - Multicast port number

        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    virtual bool Start(const std::string& multicast_address, int multicast_port);
    //! Start the server with a given multicast endpoint
    /*!
        \param multicast_endpoint - Multicast UDP endpoint

        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    virtual bool Start(const asio::ip::udp::endpoint& multicast_endpoint);
    //! Stop the server
    /*!
        \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
    */
    virtual bool Stop();
    //! Restart the server
    /*!
        \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
    */
    virtual bool Restart();

    //! Multicast datagram to the prepared mulicast endpoint (synchronous)
    /*!
        \param buffer - Datagram buffer to multicast
        \param size - Datagram buffer size
        \return Size of multicasted datagram
    */
    virtual size_t Multicast(const void* buffer, size_t size);
    //! Multicast text to the prepared mulicast endpoint (synchronous)
    /*!
        \param text - Text to multicast
        \return Size of multicasted datagram
    */
    virtual size_t Multicast(std::string_view text) { return Multicast(text.data(), text.size()); }

    //! Multicast datagram to the prepared mulicast endpoint with timeout (synchronous)
    /*!
        \param buffer - Datagram buffer to multicast
        \param size - Datagram buffer size
        \param timeout - Timeout
        \return Size of multicasted datagram
    */
    virtual size_t Multicast(const void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Multicast text to the prepared mulicast endpoint with timeout (synchronous)
    /*!
        \param text - Text to multicast
        \param timeout - Timeout
        \return Size of multicasted datagram
    */
    virtual size_t Multicast(std::string_view text, const CppCommon::Timespan& timeout) { return Multicast(text.data(), text.size(), timeout); }

    //! Multicast datagram to the prepared mulicast endpoint (asynchronous)
    /*!
        \param buffer - Datagram buffer to multicast
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully multicasted, 'false' if the datagram was not multicasted
    */
    virtual bool MulticastAsync(const void* buffer, size_t size);
    //! Multicast text to the prepared mulicast endpoint (asynchronous)
    /*!
        \param text - Text to multicast
        \return 'true' if the text was successfully multicasted, 'false' if the text was not multicasted
    */
    virtual bool MulticastAsync(std::string_view text) { return MulticastAsync(text.data(), text.size()); }

    //! Send datagram into the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send text into the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param text - Text to send
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, std::string_view text) { return Send(endpoint, text.data(), text.size()); }

    //! Send datagram into the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Send text into the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param text - Text to send
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, std::string_view text, const CppCommon::Timespan& timeout) { return Send(endpoint, text.data(), text.size(), timeout); }

    //! Send datagram into the given endpoint (asynchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully sent, 'false' if the datagram was not sent
    */
    virtual bool SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send text into the given endpoint (asynchronous)
    /*!
        \param endpoint - Endpoint to send
        \param text - Text to send
        \return 'true' if the text was successfully sent, 'false' if the text was not sent
    */
    virtual bool SendAsync(const asio::ip::udp::endpoint& endpoint, std::string_view text) { return SendAsync(endpoint, text.data(), text.size()); }

    //! Receive datagram from the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to receive from
        \param buffer - Datagram buffer to receive
        \param size - Datagram buffer size to receive
        \return Size of received datagram
    */
    virtual size_t Receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size);
    //! Receive text from the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to receive from
        \param size - Text size to receive
        \return Received text
    */
    virtual std::string Receive(asio::ip::udp::endpoint& endpoint, size_t size);

    //! Receive datagram from the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to receive from
        \param buffer - Datagram buffer to receive
        \param size - Datagram buffer size to receive
        \param timeout - Timeout
        \return Size of received datagram
    */
    virtual size_t Receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Receive text from the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to receive from
        \param size - Text size to receive
        \param timeout - Timeout
        \return Received text
    */
    virtual std::string Receive(asio::ip::udp::endpoint& endpoint, size_t size, const CppCommon::Timespan& timeout);

    //! Receive datagram from the client (asynchronous)
    virtual void ReceiveAsync();

    //! Setup option: reuse address
    /*!
        This option will enable/disable SO_REUSEADDR if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupReuseAddress(bool enable) noexcept { _option_reuse_address = enable; }
    //! Setup option: reuse port
    /*!
        This option will enable/disable SO_REUSEPORT if the OS support this feature.

        \param enable - Enable/disable option
    */
    void SetupReusePort(bool enable) noexcept { _option_reuse_port = enable; }
    //! Setup option: receive buffer limit
    /*!
        The receive operation will fail if the receive buffer limit is met.
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
        The send operation will fail if the send buffer limit is met.
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
    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle datagram received notification
    /*!
        Notification is called when another datagram was received from
        some endpoint.

        \param endpoint - Received endpoint
        \param buffer - Received datagram buffer
        \param size - Received datagram buffer size
    */
    virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) {}
    //! Handle datagram sent notification
    /*!
        Notification is called when a datagram was sent to the client.

        This handler could be used to send another datagram to the client
        for instance when the pending size is zero.

        \param endpoint - Endpoint of sent datagram
        \param sent - Size of sent datagram buffer
    */
    virtual void onSent(const asio::ip::udp::endpoint& endpoint, size_t sent) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Server Id
    CppCommon::UUID _id;
    // Asio service
    std::shared_ptr<Service> _service;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialized handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Server address, scheme & port
    std::string _address;
    int _port;
    // Server endpoint & socket
    asio::ip::udp::endpoint _endpoint;
    asio::ip::udp::socket _socket;
    std::atomic<bool> _started;
    // Server statistic
    uint64_t _bytes_sending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    uint64_t _datagrams_sent;
    uint64_t _datagrams_received;
    // Multicast, receive and send endpoints
    asio::ip::udp::endpoint _multicast_endpoint;
    asio::ip::udp::endpoint _receive_endpoint;
    asio::ip::udp::endpoint _send_endpoint;
    // Receive buffer
    bool _receiving;
    size_t _receive_buffer_limit{0};
    std::vector<uint8_t> _receive_buffer;
    HandlerStorage _receive_storage;
    // Send buffer
    bool _sending;
    size_t _send_buffer_limit{0};
    std::vector<uint8_t> _send_buffer;
    HandlerStorage _send_storage;
    // Options
    bool _option_reuse_address;
    bool _option_reuse_port;

    //! Try to receive new datagram
    void TryReceive();

    //! Clear send/receive buffers
    void ClearBuffers();

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example udp_echo_server.cpp UDP echo server example */
/*! \example udp_multicast_server.cpp UDP multicast server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_UDP_SERVER_H
