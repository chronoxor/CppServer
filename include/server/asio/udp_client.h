/*!
    \file udp_client.h
    \brief UDP client definition
    \author Ivan Shynkarenka
    \date 23.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_UDP_CLIENT_H
#define CPPSERVER_ASIO_UDP_CLIENT_H

#include "udp_resolver.h"

#include "system/uuid.h"
#include "time/timespan.h"

#include <mutex>
#include <vector>

namespace CppServer {
namespace Asio {

//! UDP client
/*!
    UDP client is used to read/write datagrams from/into the connected UDP server.

    Thread-safe.
*/
class UDPClient : public std::enable_shared_from_this<UDPClient>
{
public:
    //! Initialize UDP client with a given Asio service, server address and port number
    /*!
        \param service - Asio service
        \param address - Server address
        \param port - Server port number
    */
    UDPClient(const std::shared_ptr<Service>& service, const std::string& address, int port);
    //! Initialize UDP client with a given Asio service, server address and scheme name
    /*!
        \param service - Asio service
        \param address - Server address
        \param scheme - Scheme name
    */
    UDPClient(const std::shared_ptr<Service>& service, const std::string& address, const std::string& scheme);
    //! Initialize UDP client with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server UDP endpoint
    */
    UDPClient(const std::shared_ptr<Service>& service, const asio::ip::udp::endpoint& endpoint);
    UDPClient(const UDPClient&) = delete;
    UDPClient(UDPClient&&) = delete;
    virtual ~UDPClient() = default;

    UDPClient& operator=(const UDPClient&) = delete;
    UDPClient& operator=(UDPClient&&) = delete;

    //! Get the client Id
    const CppCommon::UUID& id() const noexcept { return _id; }

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }
    //! Get the client endpoint
    asio::ip::udp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the client socket
    asio::ip::udp::socket& socket() noexcept { return _socket; }

    //! Get the server address
    const std::string& address() const noexcept { return _address; }
    //! Get the scheme name
    const std::string& scheme() const noexcept { return _scheme; }
    //! Get the server port number
    int port() const noexcept { return _port; }

    //! Get the number of bytes pending sent by the client
    uint64_t bytes_pending() const noexcept { return _bytes_sending; }
    //! Get the number of bytes sent by the client
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the client
    uint64_t bytes_received() const noexcept { return _bytes_received; }
    //! Get the number datagrams sent by the client
    uint64_t datagrams_sent() const noexcept { return _datagrams_sent; }
    //! Get the number datagrams received by the client
    uint64_t datagrams_received() const noexcept { return _datagrams_received; }

    //! Get the option: reuse address
    bool option_reuse_address() const noexcept { return _option_reuse_address; }
    //! Get the option: reuse port
    bool option_reuse_port() const noexcept { return _option_reuse_port; }
    //! Get the option: bind the socket to the multicast UDP server
    bool option_multicast() const noexcept { return _option_multicast; }
    //! Get the option: receive buffer limit
    size_t option_receive_buffer_limit() const noexcept { return _receive_buffer_limit; }
    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer limit
    size_t option_send_buffer_limit() const noexcept { return _send_buffer_limit; }
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the client connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Connect the client (synchronous)
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool Connect();
    //! Connect the client using the given DNS resolver (synchronous)
    /*!
        \param resolver - DNS resolver
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool Connect(const std::shared_ptr<UDPResolver>& resolver);
    //! Disconnect the client (synchronous)
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool Disconnect() { return DisconnectInternal(); }
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
    //! Connect the client using the given DNS resolver (asynchronous)
    /*!
        \param resolver - DNS resolver
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool ConnectAsync(const std::shared_ptr<UDPResolver>& resolver);
    //! Disconnect the client (asynchronous)
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool DisconnectAsync() { return DisconnectInternalAsync(false); }
    //! Reconnect the client (asynchronous)
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    virtual bool ReconnectAsync();

    //! Join multicast group with a given address (synchronous)
    /*!
        \param address - Multicast group address
    */
    virtual void JoinMulticastGroup(const std::string& address);
    //! Leave multicast group with a given address (synchronous)
    /*!
        \param address - Multicast group address
    */
    virtual void LeaveMulticastGroup(const std::string& address);

    //! Join multicast group with a given address (asynchronous)
    /*!
        \param address - Multicast group address
    */
    virtual void JoinMulticastGroupAsync(const std::string& address);
    //! Leave multicast group with a given address (asynchronous)
    /*!
        \param address - Multicast group address
    */
    virtual void LeaveMulticastGroupAsync(const std::string& address);

    //! Send datagram to the connected server (synchronous)
    /*!
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return Size of sent datagram
    */
    virtual size_t Send(const void* buffer, size_t size);
    //! Send text to the connected server (synchronous)
    /*!
        \param text - Text to send
        \return Size of sent datagram
    */
    virtual size_t Send(std::string_view text) { return Send(text.data(), text.size()); }
    //! Send datagram to the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send text to the given endpoint (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param text - Text to send
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, std::string_view text) { return Send(endpoint, text.data(), text.size()); }

    //! Send datagram to the connected server with timeout (synchronous)
    /*!
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(const void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Send text to the connected server with timeout (synchronous)
    /*!
        \param text - Text to send
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(std::string_view text, const CppCommon::Timespan& timeout) { return Send(text.data(), text.size(), timeout); }
    //! Send datagram to the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size, const CppCommon::Timespan& timeout);
    //! Send text to the given endpoint with timeout (synchronous)
    /*!
        \param endpoint - Endpoint to send
        \param text - Text to send
        \param timeout - Timeout
        \return Size of sent datagram
    */
    virtual size_t Send(const asio::ip::udp::endpoint& endpoint, std::string_view text, const CppCommon::Timespan& timeout) { return Send(endpoint, text.data(), text.size(), timeout); }

    //! Send datagram to the connected server (asynchronous)
    /*!
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully sent, 'false' if the datagram was not sent
    */
    virtual bool SendAsync(const void* buffer, size_t size);
    //! Send text to the connected server (asynchronous)
    /*!
        \param text - Text to send
        \return 'true' if the text was successfully sent, 'false' if the text was not sent
    */
    virtual bool SendAsync(std::string_view text) { return SendAsync(text.data(), text.size()); }
    //! Send datagram to the given endpoint (asynchronous)
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully sent, 'false' if the datagram was not sent
    */
    virtual bool SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send text to the given endpoint (asynchronous)
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

    //! Receive datagram from the server (asynchronous)
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
    //! Setup option: bind the socket to the multicast UDP server
    /*!
        \param enable - Enable/disable option
    */
    void SetupMulticast(bool enable) noexcept { _option_reuse_address = enable; _option_multicast = enable; }
    //! Setup option: receive buffer limit
    /*!
        The client will be disconnected if the receive buffer limit is met.
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
        The client will be disconnected if the send buffer limit is met.
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
    //! Handle client connected notification
    virtual void onConnected() {}
    //! Handle client disconnected notification
    virtual void onDisconnected() {}

    //! Handle client joined multicast group notification
    /*!
        \param address - Multicast group address
    */
    virtual void onJoinedMulticastGroup(const std::string& address) {}
    //! Handle client left multicast group notification
    /*!
        \param address - Multicast group address
    */
    virtual void onLeftMulticastGroup(const std::string& address) {}

    //! Handle datagram received notification
    /*!
        Notification is called when another datagram was received
        from some endpoint.

        \param endpoint - Received endpoint
        \param buffer - Received datagram buffer
        \param size - Received datagram buffer size
    */
    virtual void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) {}
    //! Handle datagram sent notification
    /*!
        Notification is called when a datagram was sent to the server.

        This handler could be used to send another datagram to the server
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
    // Client Id
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
    std::string _scheme;
    int _port;
    // Server endpoint & client socket
    asio::ip::udp::endpoint _endpoint;
    asio::ip::udp::socket _socket;
    std::atomic<bool> _resolving;
    std::atomic<bool> _connected;
    // Client statistic
    uint64_t _bytes_sending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    uint64_t _datagrams_sent;
    uint64_t _datagrams_received;
    // Receive and send endpoints
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
    bool _option_multicast;

    //! Disconnect the client (internal synchronous)
    bool DisconnectInternal();
    //! Disconnect the client (internal asynchronous)
    bool DisconnectInternalAsync(bool dispatch);

    //! Try to receive new datagram
    void TryReceive();

    //! Clear send/receive buffers
    void ClearBuffers();

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example udp_echo_client.cpp UDP echo client example */
/*! \example udp_multicast_client.cpp UDP multicast client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_UDP_CLIENT_H
