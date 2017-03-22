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

const size_t UDPClient::CHUNK;

UDPClient::UDPClient(std::shared_ptr<Service> service, const std::string& address, int port)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(asio::ip::udp::endpoint(asio::ip::address::from_string(address), port)),
      _socket(*_service->service()),
      _connected(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _multicast(false),
      _reuse_address(false)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

UDPClient::UDPClient(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(endpoint),
      _socket(*_service->service()),
      _connected(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _multicast(false),
      _reuse_address(false)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

UDPClient::UDPClient(std::shared_ptr<Service> service, const std::string& address, int port, bool reuse_address)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(asio::ip::udp::endpoint(asio::ip::address::from_string(address), port)),
      _socket(*_service->service()),
      _connected(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _multicast(true),
      _reuse_address(reuse_address)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

UDPClient::UDPClient(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint, bool reuse_address)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(endpoint),
      _socket(*_service->service()),
      _connected(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _multicast(true),
      _reuse_address(reuse_address)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

bool UDPClient::Connect()
{
    if (IsConnected())
        return false;

    // Post the connect routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        if (IsConnected())
            return;

        // Open the client socket
        if (_multicast)
        {
            _socket.open(_endpoint.protocol()),
            _socket.set_option(asio::ip::udp::socket::reuse_address(_reuse_address));
            _socket.bind(_endpoint);
        }
        else
        {
            _socket.open(_endpoint.protocol());
            _socket.bind(asio::ip::udp::endpoint(_endpoint.protocol(), 0));
        }

        // Reset statistic
        _datagrams_sent = 0;
        _datagrams_received = 0;
        _bytes_sent = 0;
        _bytes_received = 0;

        // Update the connected flag
        _connected = true;

        // Call the client connected handler
        onConnected();

        // Try to receive something from the server
        TryReceive();
    });

    return true;
}

bool UDPClient::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    // Post the disconnect routine
    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
        if (!IsConnected())
            return;

        // Close the client socket
        _socket.close();

        // Update the connected flag
        _connected = false;

        // Call the client disconnected handler
        onDisconnected();
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        _service->Dispatch(disconnect);
    else
        _service->Post(disconnect);

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

    asio::ip::address muticast_address = asio::ip::address::from_string(address);

    // Dispatch the join multicast group routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self, muticast_address]()
    {
        if (!IsConnected())
            return;

        asio::ip::multicast::join_group join(muticast_address);
        _socket.set_option(join);
    });
}

void UDPClient::LeaveMulticastGroup(const std::string& address)
{
    if (!IsConnected())
        return;

    asio::ip::address muticast_address = asio::ip::address::from_string(address);

    // Dispatch the leave multicast group routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self, muticast_address]()
    {
        if (!IsConnected())
            return;

        asio::ip::multicast::leave_group leave(muticast_address);
        _socket.set_option(leave);
    });
}

bool UDPClient::Send(const void* buffer, size_t size)
{
    // Send the datagram to the server endpoint
    return Send(_endpoint, buffer, size);
}

bool UDPClient::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return false;

    if (!IsConnected())
        return false;

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
        Disconnect(true);
        return false;
    }

    return true;
}

void UDPClient::TryReceive()
{
    if (_reciving)
        return;

    if (!IsConnected())
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_receive_from(asio::buffer(_recive_buffer), _recive_endpoint, [this, self](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsConnected())
            return;

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            ++_datagrams_received;
            _bytes_received += size;

            // Call the datagram received handler
            onReceived(_recive_endpoint, _recive_buffer, size);
        }

        // Try to receive again if the session is valid
        if (!ec)
            TryReceive();
        else
        {
            SendError(ec);
            Disconnect(true);
        }
    });
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
