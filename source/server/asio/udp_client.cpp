/*!
    \file udp_client.cpp
    \brief UDP client implementation
    \author Ivan Shynkarenka
    \date 23.12.2016
    \copyright MIT License
*/

#include "server/asio/udp_client.h"

namespace CppServer {
namespace Asio {

UDPClient::UDPClient(std::shared_ptr<Service> service, const std::string& address, int port)
    : _id(CppCommon::UUID::Random()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _endpoint(asio::ip::udp::endpoint(asio::ip::address::from_string(address), (unsigned short)port)),
      _socket(*_io_service),
      _connected(false),
      _bytes_sending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _datagrams_sent(0),
      _datagrams_received(0),
      _receiving(false),
      _sending(false),
      _option_reuse_address(false),
      _option_reuse_port(false),
      _option_multicast(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

UDPClient::UDPClient(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _id(CppCommon::UUID::Random()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _endpoint(endpoint),
      _socket(*_io_service),
      _connected(false),
      _bytes_sending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _datagrams_sent(0),
      _datagrams_received(0),
      _receiving(false),
      _sending(false),
      _option_reuse_address(false),
      _option_reuse_port(false),
      _option_multicast(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

size_t UDPClient::option_receive_buffer_size() const
{
    asio::socket_base::receive_buffer_size option;
    _socket.get_option(option);
    return option.value();
}

size_t UDPClient::option_send_buffer_size() const
{
    asio::socket_base::send_buffer_size option;
    _socket.get_option(option);
    return option.value();
}

void UDPClient::SetupReceiveBufferSize(size_t size)
{
    asio::socket_base::receive_buffer_size option((int)size);
    _socket.set_option(option);
}

void UDPClient::SetupSendBufferSize(size_t size)
{
    asio::socket_base::send_buffer_size option((int)size);
    _socket.set_option(option);
}

bool UDPClient::Connect()
{
    if (IsConnected())
        return false;

    // Post the connect handler
    auto self(this->shared_from_this());
    auto connect_handler = [this, self]()
    {
        if (IsConnected())
            return;

        // Open a client socket
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
        if (option_multicast())
            _socket.bind(_endpoint);
        else
            _socket.bind(asio::ip::udp::endpoint(_endpoint.protocol(), 0));

        // Prepare receive buffer
        _receive_buffer.resize(option_receive_buffer_size());

        // Reset statistic
        _bytes_sending = 0;
        _bytes_sent = 0;
        _bytes_received = 0;
        _datagrams_sent = 0;
        _datagrams_received = 0;

        // Update the connected flag
        _connected = true;

        // Call the client connected handler
        onConnected();
    };
    if (_strand_required)
        _strand.post(connect_handler);
    else
        _io_service->post(connect_handler);

    return true;
}

bool UDPClient::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    // Dispatch or post the disconnect handler
    auto self(this->shared_from_this());
    auto disconnect_handler = [this, self]()
    {
        if (!IsConnected())
            return;

        // Close the client socket
        _socket.close();

        // Update the connected flag
        _connected = false;

        // Clear send/receive buffers
        ClearBuffers();

        // Call the client disconnected handler
        onDisconnected();
    };
    if (_strand_required)
    {
        if (dispatch)
            _strand.dispatch(disconnect_handler);
        else
            _strand.post(disconnect_handler);
    }
    else
    {
        if (dispatch)
            _io_service->dispatch(disconnect_handler);
        else
            _io_service->post(disconnect_handler);
    }

    return true;
}

bool UDPClient::Reconnect()
{
    if (!Disconnect())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return Connect();
}

void UDPClient::JoinMulticastGroup(const std::string& address)
{
    if (!IsConnected())
        return;

    // Dispatch the join multicast group handler
    auto self(this->shared_from_this());
    auto join_multicast_group_handler = [this, self, address]()
    {
        if (!IsConnected())
            return;

        asio::ip::address muticast_address = asio::ip::address::from_string(address);

        asio::ip::multicast::join_group join(muticast_address);
        _socket.set_option(join);

        // Call the client joined multicast group notification
        onJoinedMulticastGroup(address);
    };
    if (_strand_required)
        _strand.dispatch(join_multicast_group_handler);
    else
        _io_service->dispatch(join_multicast_group_handler);
}

void UDPClient::LeaveMulticastGroup(const std::string& address)
{
    if (!IsConnected())
        return;

    // Dispatch the leave multicast group handler
    auto self(this->shared_from_this());
    auto leave_multicast_group_handler = [this, self, address]()
    {
        if (!IsConnected())
            return;

        asio::ip::address muticast_address = asio::ip::address::from_string(address);

        asio::ip::multicast::leave_group leave(muticast_address);
        _socket.set_option(leave);

        // Call the client left multicast group notification
        onLeftMulticastGroup(address);
    };
    if (_strand_required)
        _strand.dispatch(leave_multicast_group_handler);
    else
        _io_service->dispatch(leave_multicast_group_handler);
}

void UDPClient::Receive()
{
    // Try to receive something from the server
    TryReceive();
}

bool UDPClient::SendAsync(const void* buffer, size_t size)
{
    // Send the datagram to the server endpoint
    return SendAsync(_endpoint, buffer, size);
}

bool UDPClient::SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    if (_sending)
        return false;

    if (!IsConnected())
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

        if (!IsConnected())
            return;

        // Disconnect on error
        if (ec)
        {
            SendError(ec);
            Disconnect(true);
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

bool UDPClient::SendSync(const void* buffer, size_t size)
{
    // Send the datagram to the server endpoint
    return SendSync(_endpoint, buffer, size);
}

bool UDPClient::SendSync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    if (!IsConnected())
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

    // Disconnect on error
    if (ec)
    {
        SendError(ec);
        Disconnect(true);
        return false;
    }

    return true;
}

void UDPClient::TryReceive()
{
    if (_receiving)
        return;

    if (!IsConnected())
        return;

    // Async receive with the receive handler
    _receiving = true;
    auto self(this->shared_from_this());
    auto async_receive_handler = make_alloc_handler(_receive_storage, [this, self](std::error_code ec, size_t size)
    {
        _receiving = false;

        if (!IsConnected())
            return;

        // Disconnect on error
        if (ec)
        {
            SendError(ec);
            Disconnect(true);
            return;
        }

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            ++_datagrams_received;
            _bytes_received += size;

            // If the receive buffer is full increase its size
            if (_receive_buffer.size() == size)
                _receive_buffer.resize(2 * size);

            // Call the datagram received handler
            onReceived(_receive_endpoint, _receive_buffer.data(), size);
        }
    });
    if (_strand_required)
        _socket.async_receive_from(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), _receive_endpoint, bind_executor(_strand, async_receive_handler));
    else
        _socket.async_receive_from(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), _receive_endpoint, async_receive_handler);
}

void UDPClient::ClearBuffers()
{
    // Clear send buffers
    _send_buffer.clear();

    // Update statistic
    _bytes_sending = 0;
}

void UDPClient::SendError(std::error_code ec)
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
