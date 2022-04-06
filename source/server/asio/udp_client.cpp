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

UDPClient::UDPClient(const std::shared_ptr<Service>& service, const std::string& address, int port)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _address(address),
      _port(port),
      _socket(*_io_service),
      _resolving(false),
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

UDPClient::UDPClient(const std::shared_ptr<Service>& service, const std::string& address, const std::string& scheme)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _address(address),
      _scheme(scheme),
      _port(0),
      _socket(*_io_service),
      _resolving(false),
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

UDPClient::UDPClient(const std::shared_ptr<Service>& service, const asio::ip::udp::endpoint& endpoint)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _address(endpoint.address().to_string()),
      _port(endpoint.port()),
      _endpoint(endpoint),
      _socket(*_io_service),
      _resolving(false),
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
    assert(!_address.empty() && "Server address must not be empty!");
    if (_address.empty())
        return false;
    assert((_port > 0) && "Server port number must be valid!");
    if (_port <= 0)
        return false;

    if (IsConnected())
        return false;

    // Create the server endpoint
    _endpoint = asio::ip::udp::endpoint(asio::ip::make_address(_address), (unsigned short)_port);

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

    return true;
}

bool UDPClient::Connect(const std::shared_ptr<UDPResolver>& resolver)
{
    assert((resolver != nullptr) && "UDP resolver is invalid!");
    if (resolver == nullptr)
        return false;
    assert(!_address.empty() && "Server address must not be empty!");
    if (_address.empty())
        return false;
    assert((_port > 0) && "Server port number must be valid!");
    if (_port <= 0)
        return false;

    if (IsConnected())
        return false;

    std::error_code ec;

    // Resolve the server endpoint
    asio::ip::udp::resolver::query query(_address, (_scheme.empty() ? std::to_string(_port) : _scheme));
    auto endpoints = resolver->resolver().resolve(query, ec);

    // Check for resolve errors
    if (ec)
    {
        SendError(ec);

        // Call the client disconnected handler
        onDisconnected();

        return false;
    }

    _endpoint = *endpoints;

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

    return true;
}

bool UDPClient::DisconnectInternal()
{
    if (!IsConnected())
        return false;

    // Close the client socket
    _socket.close();

    // Update the connected flag
    _resolving = false;
    _connected = false;

    // Update sending/receiving flags
    _receiving = false;
    _sending = false;

    // Clear send/receive buffers
    ClearBuffers();

    // Call the client disconnected handler
    onDisconnected();

    return true;
}

bool UDPClient::Reconnect()
{
    if (!Disconnect())
        return false;

    return Connect();
}

bool UDPClient::ConnectAsync()
{
    if (IsConnected())
        return false;

    // Post the connect handler
    auto self(this->shared_from_this());
    auto connect_handler = [this, self]() { Connect(); };
    if (_strand_required)
        _strand.post(connect_handler);
    else
        _io_service->post(connect_handler);

    return true;
}

bool UDPClient::ConnectAsync(const std::shared_ptr<UDPResolver>& resolver)
{
    assert((resolver != nullptr) && "UDP resolver is invalid!");
    if (resolver == nullptr)
        return false;

    if (IsConnected() || _resolving)
        return false;

    // Post the connect handler
    auto self(this->shared_from_this());
    auto connect_handler = [this, self, resolver]()
    {
        if (IsConnected() || _resolving)
            return;

        // Async DNS resolve with the resolve handler
        _resolving = true;
        auto async_resolve_handler = [this, self](std::error_code ec, asio::ip::udp::resolver::results_type endpoints)
        {
            _resolving = false;

            if (IsConnected() || _resolving)
                return;

            if (!ec)
            {
                // Resolve the server endpoint
                _endpoint = *endpoints;

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
            }
            else
            {
                SendError(ec);

                // Call the client disconnected handler
                onDisconnected();
            }
        };

        // Resolve the server endpoint
        asio::ip::udp::resolver::query query(_address, (_scheme.empty() ? std::to_string(_port) : _scheme));
        if (_strand_required)
            resolver->resolver().async_resolve(query, bind_executor(_strand, async_resolve_handler));
        else
            resolver->resolver().async_resolve(query, async_resolve_handler);
    };
    if (_strand_required)
        _strand.post(connect_handler);
    else
        _io_service->post(connect_handler);

    return true;
}

bool UDPClient::DisconnectInternalAsync(bool dispatch)
{
    if (!IsConnected())
        return false;

    asio::error_code ec;

    // Cancel the client socket
    _socket.cancel(ec);

    // Dispatch or post the disconnect handler
    auto self(this->shared_from_this());
    auto disconnect_handler = [this, self]() { DisconnectInternal(); };
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

bool UDPClient::ReconnectAsync()
{
    if (!DisconnectAsync())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return ConnectAsync();
}

void UDPClient::JoinMulticastGroup(const std::string& address)
{
    if (!IsConnected())
        return;

    asio::ip::address muticast_address = asio::ip::make_address(address);

    asio::ip::multicast::join_group join(muticast_address);
    _socket.set_option(join);

    // Call the client joined multicast group notification
    onJoinedMulticastGroup(address);
}

void UDPClient::LeaveMulticastGroup(const std::string& address)
{
    if (!IsConnected())
        return;

    asio::ip::address muticast_address = asio::ip::make_address(address);

    asio::ip::multicast::leave_group leave(muticast_address);
    _socket.set_option(leave);

    // Call the client left multicast group notification
    onLeftMulticastGroup(address);
}

void UDPClient::JoinMulticastGroupAsync(const std::string& address)
{
    if (!IsConnected())
        return;

    // Dispatch the join multicast group handler
    auto self(this->shared_from_this());
    auto join_multicast_group_handler = [this, self, address]() { JoinMulticastGroup(address); };
    if (_strand_required)
        _strand.dispatch(join_multicast_group_handler);
    else
        _io_service->dispatch(join_multicast_group_handler);
}

void UDPClient::LeaveMulticastGroupAsync(const std::string& address)
{
    if (!IsConnected())
        return;

    // Dispatch the leave multicast group handler
    auto self(this->shared_from_this());
    auto leave_multicast_group_handler = [this, self, address]() { LeaveMulticastGroup(address); };
    if (_strand_required)
        _strand.dispatch(leave_multicast_group_handler);
    else
        _io_service->dispatch(leave_multicast_group_handler);
}

size_t UDPClient::Send(const void* buffer, size_t size)
{
    // Send the datagram to the server endpoint
    return Send(_endpoint, buffer, size);
}

size_t UDPClient::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    if (!IsConnected())
        return 0;

    if (size == 0)
        return 0;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
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

    // Disconnect on error
    if (ec)
    {
        SendError(ec);
        Disconnect();
    }

    return sent;
}

size_t UDPClient::Send(const void* buffer, size_t size, const CppCommon::Timespan& timeout)
{
    // Send the datagram to the server endpoint
    return Send(_endpoint, buffer, size, timeout);
}

size_t UDPClient::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size, const CppCommon::Timespan& timeout)
{
    if (!IsConnected())
        return 0;

    if (size == 0)
        return 0;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return 0;

    int done = 0;
    std::mutex mtx;
    std::condition_variable cv;
    asio::error_code error;
    asio::system_timer timer(_socket.get_executor());

    // Prepare done handler
    auto async_done_handler = [&](asio::error_code ec)
    {
        std::unique_lock<std::mutex> lck(mtx);
        if (done++ == 0)
        {
            error = ec;
            _socket.cancel();
            timer.cancel();
        }
        cv.notify_one();
    };

    // Async wait for timeout
    timer.expires_from_now(timeout.chrono());
    timer.async_wait([&](const asio::error_code& ec) { async_done_handler(ec ? ec : asio::error::timed_out); });

    // Async send datagram to the server
    size_t sent = 0;
    _socket.async_send_to(asio::buffer(buffer, size), endpoint, [&](std::error_code ec, size_t write) { async_done_handler(ec); sent = write; });

    // Wait for complete or timeout
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [&]() { return done == 2; });

    // Send datagram to the server
    if (sent > 0)
    {
        // Update statistic
        ++_datagrams_sent;
        _bytes_sent += sent;

        // Call the datagram sent handler
        onSent(endpoint, sent);
    }

    // Disconnect on error
    if (error && (error != asio::error::timed_out))
    {
        SendError(error);
        Disconnect();
    }

    return sent;
}

bool UDPClient::SendAsync(const void* buffer, size_t size)
{
    // Send the datagram to the server endpoint
    return SendAsync(_endpoint, buffer, size);
}

bool UDPClient::SendAsync(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    if (_sending)
        return false;

    if (!IsConnected())
        return false;

    if (size == 0)
        return true;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    // Check the send buffer limit
    if ((size > _send_buffer_limit) && (_send_buffer_limit > 0))
    {
        SendError(asio::error::no_buffer_space);
        return false;
    }

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
            DisconnectInternalAsync(true);
            return;
        }

        // Send some data to the server
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

size_t UDPClient::Receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size)
{
    if (!IsConnected())
        return 0;

    if (size == 0)
        return 0;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return 0;

    asio::error_code ec;

    // Receive datagram from the server
    size_t received = _socket.receive_from(asio::buffer(buffer, size), endpoint, 0, ec);

    // Update statistic
    ++_datagrams_received;
    _bytes_received += received;

    // Call the datagram received handler
    onReceived(endpoint, buffer, received);

    // Disconnect on error
    if (ec)
    {
        SendError(ec);
        Disconnect();
    }

    return received;
}

std::string UDPClient::Receive(asio::ip::udp::endpoint& endpoint, size_t size)
{
    std::string text(size, 0);
    text.resize(Receive(endpoint, text.data(), text.size()));
    return text;
}

size_t UDPClient::Receive(asio::ip::udp::endpoint& endpoint, void* buffer, size_t size, const CppCommon::Timespan& timeout)
{
    if (!IsConnected())
        return 0;

    if (size == 0)
        return 0;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return 0;

    int done = 0;
    std::mutex mtx;
    std::condition_variable cv;
    asio::error_code error;
    asio::system_timer timer(_socket.get_executor());

    // Prepare done handler
    auto async_done_handler = [&](asio::error_code ec)
    {
        std::unique_lock<std::mutex> lck(mtx);
        if (done++ == 0)
        {
            error = ec;
            _socket.cancel();
            timer.cancel();
        }
        cv.notify_one();
    };

    // Async wait for timeout
    timer.expires_from_now(timeout.chrono());
    timer.async_wait([&](const asio::error_code& ec) { async_done_handler(ec ? ec : asio::error::timed_out); });

    // Async receive datagram from the server
    size_t received = 0;
    _socket.async_receive_from(asio::buffer(buffer, size), endpoint, [&](std::error_code ec, size_t read) { async_done_handler(ec); received = read; });

    // Wait for complete or timeout
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [&]() { return done == 2; });

    // Update statistic
    ++_datagrams_received;
    _bytes_received += received;

    // Call the datagram received handler
    onReceived(endpoint, buffer, received);

    // Disconnect on error
    if (error && (error != asio::error::timed_out))
    {
        SendError(error);
        Disconnect();
    }

    return received;
}

std::string UDPClient::Receive(asio::ip::udp::endpoint& endpoint, size_t size, const CppCommon::Timespan& timeout)
{
    std::string text(size, 0);
    text.resize(Receive(endpoint, text.data(), text.size(), timeout));
    return text;
}

void UDPClient::ReceiveAsync()
{
    // Try to receive datagrams from the server
    TryReceive();
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
            DisconnectInternalAsync(true);
            return;
        }

        // Update statistic
        ++_datagrams_received;
        _bytes_received += size;

        // Call the datagram received handler
        onReceived(_receive_endpoint, _receive_buffer.data(), size);

        // If the receive buffer is full increase its size
        if (_receive_buffer.size() == size)
        {
            // Check the receive buffer limit
            if (((2 * size) > _receive_buffer_limit) && (_receive_buffer_limit > 0))
            {
                SendError(asio::error::no_buffer_space);
                DisconnectInternalAsync(true);
                return;
            }

            _receive_buffer.resize(2 * size);
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
