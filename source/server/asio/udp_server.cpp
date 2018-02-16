/*!
    \file udp_server.cpp
    \brief UDP server implementation
    \author Ivan Shynkarenka
    \date 22.12.2016
    \copyright MIT License
*/

#include "server/asio/udp_server.h"

namespace CppServer {
namespace Asio {

UDPServer::UDPServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port)
    : _service(service),
      _socket(*_service->service()),
      _started(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _recive_buffer(CHUNK + 1),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    switch (protocol)
    {
        case InternetProtocol::IPv4:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), (unsigned short)port);
            break;
        case InternetProtocol::IPv6:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v6(), (unsigned short)port);
            break;
    }
}

UDPServer::UDPServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _socket(*_service->service()),
      _started(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _recive_buffer(CHUNK + 1),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    _endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(address), (unsigned short)port);
}

UDPServer::UDPServer(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _socket(*_service->service()),
      _started(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _recive_buffer(CHUNK + 1),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

bool UDPServer::Start()
{
    assert(!IsStarted() && "UDP server is already started!");
    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        if (IsStarted())
            return;

        // Open a server socket
        _socket.open(_endpoint.protocol());
        _socket.set_option(asio::ip::udp::socket::reuse_address(option_reuse_address()));
#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)
        //typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
        //_socket.set_option(reuse_port(option_reuse_port()));
#endif
        _socket.bind(_endpoint);

        // Reset statistic
        _datagrams_sent = 0;
        _datagrams_received = 0;
        _bytes_sent = 0;
        _bytes_received = 0;

         // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Try to receive datagrams from the clients
        TryReceive();
    });

    return true;
}

bool UDPServer::Start(const std::string& multicast_address, int multicast_port)
{
    _multicast_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(multicast_address), (unsigned short)multicast_port);
    return Start();
}

bool UDPServer::Start(const asio::ip::udp::endpoint& multicast_endpoint)
{
    _multicast_endpoint = multicast_endpoint;
    return Start();
}

bool UDPServer::Stop()
{
    assert(IsStarted() && "UDP server is not started!");
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        if (!IsStarted())
            return;

        // Close the server socket
        _socket.close();

        // Update the started flag
        _started = false;

        // Call the server stopped handler
        onStopped();
    });

    return true;
}

bool UDPServer::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

bool UDPServer::Multicast(const void* buffer, size_t size)
{
    // Send the datagram to the multicast endpoint
    return Send(_multicast_endpoint, buffer, size);
}

bool UDPServer::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsStarted())
        return 0;

    asio::error_code ec;

    // Sent datagram to the server
    size_t sent = _socket.send_to(asio::const_buffer(buffer, size), endpoint, 0, ec);
    if (sent > 0)
    {
        // Update statistic
        ++_datagrams_sent;
        _bytes_sent += sent;

        // Call the datagram sent handler
        onSent(endpoint, sent);
    }

    // Check for error
    if (ec)
    {
        SendError(ec);
        return false;
    }

    return true;
}

void UDPServer::TryReceive()
{
    if (_reciving)
        return;

    if (!IsStarted())
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_receive_from(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), _recive_endpoint, [this, self](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsStarted())
            return;

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            ++_datagrams_received;
            _bytes_received += size;

            // If the receive buffer is full increase its size
            if (_recive_buffer.size() == size)
                _recive_buffer.resize(2 * size);

            // Call the datagram received handler
            onReceived(_recive_endpoint, _recive_buffer.data(), size);
        }

        // Try to receive again if the session is valid
        if (!ec)
            TryReceive();
        else
            SendError(ec);
    });
}

void UDPServer::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
