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
    //! Initialize UDP server with a given Asio service, protocol and port number
    /*!
        \param service - Asio service
        \param protocol - Protocol type
        \param port - Port number
    */
    explicit UDPServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port);
    //! Initialize UDP server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit UDPServer(std::shared_ptr<Service> service, const std::string& address, int port);
    //! Initialize UDP server with a given Asio service and endpoint
    /*!
        \param service - Asio service
        \param endpoint - Server UDP endpoint
    */
    explicit UDPServer(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint);
    UDPServer(const UDPServer&) = delete;
    UDPServer(UDPServer&&) = default;
    virtual ~UDPServer() = default;

    UDPServer& operator=(const UDPServer&) = delete;
    UDPServer& operator=(UDPServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the server endpoint
    asio::ip::udp::endpoint& endpoint() noexcept { return _endpoint; }
    //! Get the server multicast endpoint
    asio::ip::udp::endpoint& multicast_endpoint() noexcept { return _multicast_endpoint; }

    //! Get the number datagrams sent by this server
    uint64_t datagrams_sent() const noexcept { return _datagrams_sent; }
    //! Get the number datagrams received by this server
    uint64_t datagrams_received() const noexcept { return _datagrams_received; }
    //! Get the number of bytes sent by this server
    uint64_t bytes_sent() const noexcept { return _bytes_sent; }
    //! Get the number of bytes received by this server
    uint64_t bytes_received() const noexcept { return _bytes_received; }

    //! Is the server started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start();
    //! Start the server with a given multicast IP address and port number
    /*!
        \param multicast_address - Multicast IP address
        \param multicast_port - Multicast port number

        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start(const std::string& multicast_address, int multicast_port);
    //! Start the server with a given multicast endpoint
    /*!
        \param multicast_endpoint - Multicast UDP endpoint

        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start(const asio::ip::udp::endpoint& multicast_endpoint);
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

    //! Multicast a datagram to the prepared mulicast endpoint
    /*!
        \param buffer - Datagram buffer to multicast
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully multicasted, 'false' if the datagram was not multicasted
    */
    bool Multicast(const void* buffer, size_t size);
    //! Multicast a text string to the prepared mulicast endpoint
    /*!
        \param text - Text string to multicast
        \return 'true' if the datagram was successfully multicasted, 'false' if the datagram was not multicasted
    */
    bool Multicast(const std::string& text) { return Multicast(text.data(), text.size()); }

    //! Send a datagram into the given endpoint
    /*!
        \param endpoint - Endpoint to send
        \param buffer - Datagram buffer to send
        \param size - Datagram buffer size
        \return 'true' if the datagram was successfully multicasted, 'false' if the datagram was not multicasted
    */
    bool Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size);
    //! Send a text string into the given endpoint
    /*!
        \param endpoint - Endpoint to send
        \param text - Text string to send
        \return 'true' if the datagram was successfully multicasted, 'false' if the datagram was not multicasted
    */
    bool Send(const asio::ip::udp::endpoint& endpoint, const std::string& text) { return Send(endpoint, text.data(), text.size()); }

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
    // Asio service
    std::shared_ptr<Service> _service;
    // Server endpoint & socket
    asio::ip::udp::endpoint _endpoint;
    asio::ip::udp::socket _socket;
    std::atomic<bool> _started;
    // Server statistic
    uint64_t _datagrams_sent;
    uint64_t _datagrams_received;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Multicast & receive endpoint
    asio::ip::udp::endpoint _multicast_endpoint;
    asio::ip::udp::endpoint _recive_endpoint;
    // Receive & send buffers
    std::vector<uint8_t> _recive_buffer;
    bool _reciving;

    static const size_t CHUNK = 8192;

    //! Try to receive new datagram
    void TryReceive();

    //! Clear receive & send buffers
    void ClearBuffers();
};

/*! \example udp_echo_server.cpp UDP echo server example */
/*! \example udp_multicast_server.cpp UDP multicast server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_UDP_SERVER_H
