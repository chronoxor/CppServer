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
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _socket(*_io_service),
      _started(false),
      _bytes_sending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _datagrams_sent(0),
      _datagrams_received(0),
      _receiving(false),
      _sending(false),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    // Prepare endpoint
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
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _socket(*_io_service),
      _started(false),
      _bytes_sending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _datagrams_sent(0),
      _datagrams_received(0),
      _receiving(false),
      _sending(false),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    // Prepare endpoint
    _endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(address), (unsigned short)port);
}

UDPServer::UDPServer(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _endpoint(endpoint),
      _socket(*_io_service),
      _started(false),
      _bytes_sending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _datagrams_sent(0),
      _datagrams_received(0),
      _receiving(false),
      _sending(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

size_t UDPServer::option_receive_buffer_size() const
{
    asio::socket_base::receive_buffer_size option;
    _socket.get_option(option);
    return option.value();
}

size_t UDPServer::option_send_buffer_size() const
{
    asio::socket_base::send_buffer_size option;
    _socket.get_option(option);
    return option.value();
}

void UDPServer::SetupReceiveBufferSize(size_t size)
{
    asio::socket_base::receive_buffer_size option((int)size);
    _socket.set_option(option);
}

void UDPServer::SetupSendBufferSize(size_t size)
{
    asio::socket_base::send_buffer_size option((int)size);
    _socket.set_option(option);
}

bool UDPServer::StartAsync()
{
    assert(!IsStarted() && "UDP server is already started!");
    if (IsStarted())
        return false;

    // Post the start handler
    auto self(this->shared_from_this());
    auto start_handler = [this, self]()
    {
        if (IsStarted())
            return;

        // Open a server socket
        _socket.open(_endpoint.protocol());
        if (option_reuse_address())
            _socket.set_option(asio::ip::udp::socket::reuse_address(true));
#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)
        if (option_reuse_port())
        {
            typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
            _socket.set_option(reuse_port(true));
        }
#endif
        _socket.bind(_endpoint);

        // Prepare receive buffer
        _receive_buffer.resize(option_receive_buffer_size());

        // Reset statistic
        _bytes_sending = 0;
        _bytes_sent = 0;
        _bytes_received = 0;
        _datagrams_sent = 0;
        _datagrams_received = 0;

         // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();
    };
    if (_strand_required)
        _strand.post(start_handler);
    else
        _io_service->post(start_handler);

    return true;
}

bool UDPServer::StartAsync(const std::string& multicast_address, int multicast_port)
{
    _multicast_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(multicast_address), (unsigned short)multicast_port);
    return StartAsync();
}

bool UDPServer::StartAsync(const asio::ip::udp::endpoint& multicast_endpoint)
{
    _multicast_endpoint = multicast_endpoint;
    return StartAsync();
}

bool UDPServer::StopAsync()
{
    assert(IsStarted() && "UDP server is not started!");
    if (!IsStarted())
        return false;

    // Post the stop handler
    auto self(this->shared_from_this());
    auto stop_handler = [this, self]()
    {
        if (!IsStarted())
            return;

        // Close the server socket
        _socket.close();

        // Update the started flag
        _started = false;

        // Update sending/receiving flags
        _receiving = false;
        _sending = false;

        // Clear send/receive buffers
        ClearBuffers();

        // Call the server stopped handler
        onStopped();
    };
    if (_strand_required)
        _strand.post(stop_handler);
    else
        _io_service->post(stop_handler);

    return true;
}

bool UDPServer::RestartAsync()
{
    if (!StopAsync())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return StartAsync();
}

bool UDPServer::Multicast(const void* buffer, size_t size)
{
    // Send the datagram to the multicast endpoint
    return Send(_multicast_endpoint, buffer, size);
}

bool UDPServer::MulticastAsync(const void* buffer, size_t size)
{
    // Send the datagram to the multicast endpoint
    return SendAsync(_multicast_endpoint, buffer, size);
}

bool UDPServer::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    if (!IsStarted())
        return false;

    if (size == 0)
        return true;

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

bool UDPServer::SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    if (_sending)
        return false;

    if (!IsStarted())
        return false;

    if (size == 0)
        return true;

    // Fill the main send buffer
    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.assign(bytes, bytes + size);

    // Update statistic
    _bytes_sending = _send_buffer.size();

    // Update send endpoint
    _send_endpoint = endpoint;

    // Async send-to with the send-to handler
    _sending = true;
    auto self(this->shared_from_this());
    auto async_send_to_handler = make_alloc_handler(_send_storage, [this, self](std::error_code ec, size_t sent)
    {
        _sending = false;

        if (!IsStarted())
            return;

        // Check for error
        if (ec)
        {
            SendError(ec);
            return;
        }

        // Send some data to the client
        if (sent > 0)
        {
            // Update statistic
            _bytes_sending = 0;
            _bytes_sent += sent;

            // Clear the send buffer
            _send_buffer.clear();

            // Call the buffer sent handler
            onSent(_send_endpoint, sent);
        }
    });
    if (_strand_required)
        _socket.async_send_to(asio::buffer(_send_buffer.data(), _send_buffer.size()), _send_endpoint, bind_executor(_strand, async_send_to_handler));
    else
        _socket.async_send_to(asio::buffer(_send_buffer.data(), _send_buffer.size()), _send_endpoint, async_send_to_handler);

    return true;
}

size_t UDPServer::Receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return 0;

    if (!IsStarted())
        return 0;

    if (size == 0)
        return 0;

    asio::error_code ec;

    // Receive datagram from the client
    size_t received = _socket.receive_from(asio::buffer(buffer, size), endpoint, 0, ec);
    if (received > 0)
    {
        // Update statistic
        ++_datagrams_received;
        _bytes_received += received;

        // Call the datagram received handler
        onReceived(endpoint, buffer, received);
    }

    // Check for error
    if (ec)
    {
        SendError(ec);
        return received;
    }

    return received;
}

std::string UDPServer::Receive(asio::ip::udp::endpoint& endpoint, size_t size)
{
    std::string text(size, 0);
    text.resize(Receive(endpoint, text.data(), text.size()));
    return text;
}

void UDPServer::ReceiveAsync()
{
    // Try to receive datagrams from clients
    TryReceive();
}

void UDPServer::TryReceive()
{
    if (_receiving)
        return;

    if (!IsStarted())
        return;

    // Async receive with the receive handler
    _receiving = true;
    auto self(this->shared_from_this());
    auto async_receive_handler = make_alloc_handler(_receive_storage, [this, self](std::error_code ec, size_t size)
    {
        _receiving = false;

        if (!IsStarted())
            return;

        // Check for error
        if (ec)
        {
            SendError(ec);
            return;
        }

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            ++_datagrams_received;
            _bytes_received += size;

            // Call the datagram received handler
            onReceived(_receive_endpoint, _receive_buffer.data(), size);

            // If the receive buffer is full increase its size
            if (_receive_buffer.size() == size)
                _receive_buffer.resize(2 * size);
        }
    });
    if (_strand_required)
        _socket.async_receive_from(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), _receive_endpoint, bind_executor(_strand, async_receive_handler));
    else
        _socket.async_receive_from(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), _receive_endpoint, async_receive_handler);
}

void UDPServer::ClearBuffers()
{
    // Clear send buffers
    _send_buffer.clear();

    // Update statistic
    _bytes_sending = 0;
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
