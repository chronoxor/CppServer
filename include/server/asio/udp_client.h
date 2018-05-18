/*!
    \file udp_client.h
    \brief UDP client definition
    \author Ivan Shynkarenka
    \date 23.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_UDP_CLIENT_H
#define CPPSERVER_ASIO_UDP_CLIENT_H

#include "service.h"

#include "system/uuid.h"

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
    //! Initialize UDP client with a given Asio service, server IP address and port number
    /*!
        \param service - Asio service
        \param address - Server IP address
        \param port - Server port number
    */
    UDPClient(std::shared_ptr<Service> service, const std::string& address, int port);
    //! Initialize UDP client with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server UDP endpoint
    */
    UDPClient(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint);
    UDPClient(const UDPClient&) = delete;
    UDPClient(UDPClient&&) = default;
    virtual ~UDPClient() = default;

    UDPClient& operator=(const UDPClient&) = delete;
    UDPClient& operator=(UDPClient&&) = default;

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

    //! Get the number datagrams sent by the client
    uint64_t datagrams_sent() const noexcept { return _datagrams_sent; }
    //! Get the number datagrams received by the client
    uint64_t datagrams_received() const noexcept { return _datagrams_received; }
    //! Get the number of bytes pending sent by the client
    uint64_t bytes_pending() const noexcept { return 0; }
    //! Get the number of bytes sent by the client
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by the client
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Get the option: reuse address
    bool option_reuse_address() const noexcept { return _option_reuse_address; }
    //! Get the option: reuse port
    bool option_reuse_port() const noexcept { return _option_reuse_port; }
    //! Get the option: bind the socket to the multicast UDP server
    bool option_multicast() const noexcept { return _option_multicast; }
    //! Get the option: receive buffer size
    size_t option_receive_buffer_size() const;
    //! Get the option: send buffer size
    size_t option_send_buffer_size() const;

    //! Is the client connected?
    bool IsConnected() const noexcept { return _connected; }

    //! Connect the client
    /*!
        \return 'true' if the client was successfully connected, 'false' if the client failed to connect
    */
    virtual bool Connect();
    //! Disconnect the client
    /*!
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    virtual bool Disconnect() { return Disconnect(false); }
    //! Reconnect the client
    /*!
        \return 'true' if the client was successfully reconnected, 'false' if the client is already reconnected
    */
    virtual bool Reconnect();

    //! Join multicast group with a given IP address
    /*!
        \param address - IP address
    */
    virtual void JoinMulticastGroup(const std::string& address);
    //! Leave multicast group with a given IP address
    /*!
        \param address - IP address
    */
    virtual void LeaveMulticastGroup(const std::string& address);

    //! Send datagram to the connected server
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the datagram was successfully sent, 'false' if the datagram was not sent
    */
    virtual bool Send(const void* buffer, size_t size);
    //! Send text to the connected server
    /*!
        \param text - Text string to send
        \return 'true' if the text was successfully sent, 'false' if the text was not sent
    */
    virtual bool Send(const std::string& text) { return Send(text.data(), text.size()); }

    //! Send datagram to the given endpoint
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Buffer to send
        \param size - Buffer size
        \return 'true' if the datagram was successfully sent, 'false' if the datagram was not sent
    */
    virtual bool Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send text to the given endpoint
    /*!
        \param endpoint - Endpoint to send
        \param text - Text string to send
        \return 'true' if the text was successfully sent, 'false' if the text was not sent
    */
    virtual bool Send(const asio::ip::udp::endpoint& endpoint, const std::string& text) { return Send(endpoint, text.data(), text.size()); }

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

    //! Handle client joined multicast group notification
    /*!
        \param address - IP address
    */
    virtual void onJoinedMulticastGroup(const std::string& address) {}
    //! Handle client left multicast group notification
    /*!
        \param address - IP address
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
    // Server endpoint & client socket
    asio::ip::udp::endpoint _endpoint;
    asio::ip::udp::socket _socket;
    std::atomic<bool> _connected;
    // Client statistic
    uint64_t _datagrams_sent;
    uint64_t _datagrams_received;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive endpoint
    asio::ip::udp::endpoint _recive_endpoint;
    // Receive buffer
    bool _reciving;
    std::vector<uint8_t> _recive_buffer;
    HandlerStorage _recive_storage;
    // Options
    bool _option_reuse_address;
    bool _option_reuse_port;
    bool _option_multicast;

    //! Disconnect the client
    /*!
        \param dispatch - Dispatch flag
        \return 'true' if the client was successfully disconnected, 'false' if the client is already disconnected
    */
    bool Disconnect(bool dispatch);

    //! Try to receive new datagram
    void TryReceive();

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example udp_echo_client.cpp UDP echo client example */
/*! \example udp_multicast_client.cpp UDP multicast client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_UDP_CLIENT_H
