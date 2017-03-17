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
      _reciving(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    switch (protocol)
    {
        case InternetProtocol::IPv4:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), port);
            break;
        case InternetProtocol::IPv6:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v6(), port);
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
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    _endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(address), port);
}

UDPServer::UDPServer(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _socket(*_service->service()),
      _started(false),
      _datagrams_sent(0),
      _datagrams_received(0),
      _bytes_sent(0),
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
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
        // Open the server socket
        _socket = asio::ip::udp::socket(*_service->service(), _endpoint);

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
    _multicast_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(multicast_address), multicast_port);
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
        // Close the server socket
        _socket.close();

        // Clear receive/send buffers
        ClearBuffers();

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

    // Prepare receive buffer
    size_t old_size = _recive_buffer.size();
    _recive_buffer.resize(_recive_buffer.size() + CHUNK);

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_receive_from(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), _recive_endpoint, [this, self](std::error_code ec, size_t received)
    {
        _reciving = false;

        // Check if the server is stopped
        if (!IsStarted())
            return;

        // Received datagram from the client
        if (received > 0)
        {
            // Update statistic
            ++_datagrams_received;
            _bytes_received += received;

            // Prepare receive buffer
            _recive_buffer.resize(_recive_buffer.size() - (CHUNK - received));

            // Call the datagram received handler
            onReceived(_recive_endpoint, _recive_buffer.data(), _recive_buffer.size());

            // Clear the handled buffer
            _recive_buffer.clear();
        }

        // Try to receive again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            service()->Post([this, self]() { TryReceive(); });
        else
            SendError(ec);
    });
}

void UDPServer::ClearBuffers()
{
    _recive_buffer.clear();
}

void UDPServer::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof))
        return;

    // Skip OpenSSL annoying errors
    if (ec == asio::ssl::error::stream_truncated)
        return;
    if (ec.category() == asio::error::get_ssl_category())
    {
        if ((ERR_GET_REASON(ec.value()) == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC) ||
            (ERR_GET_REASON(ec.value()) == SSL_R_PROTOCOL_IS_SHUTDOWN) ||
            (ERR_GET_REASON(ec.value()) == SSL_R_WRONG_VERSION_NUMBER))
            return;
    }

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
