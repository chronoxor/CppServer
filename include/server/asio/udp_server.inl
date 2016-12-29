/*!
    \file udp_server.inl
    \brief UDP server inline implementation
    \author Ivan Shynkarenka
    \date 22.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

inline UDPServer::UDPServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port)
    : _service(service),
      _socket(_service->service()),
      _started(false),
      _reciving(false),
      _sending(false)
{
    switch (protocol)
    {
        case InternetProtocol::IPv4:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v4(), port);
            break;
        case InternetProtocol::IPv6:
            _endpoint = asio::ip::udp::endpoint(asio::ip::udp::v6(), port);
            break;
    }
    _socket = asio::ip::udp::socket(_service->service(), _endpoint);
}

inline UDPServer::UDPServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _socket(_service->service()),
      _started(false)
{
    _endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(address), port);
    _socket = asio::ip::udp::socket(_service->service(), _endpoint);
}

inline UDPServer::UDPServer(std::shared_ptr<Service> service, const asio::ip::udp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _socket(_service->service(), endpoint),
      _started(false)
{
}

inline bool UDPServer::Start()
{
    if (!_service->IsStarted())
        return false;

    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
         // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Try to receive datagrams from the clients
        TryReceive();
    });

    return true;
}

inline bool UDPServer::Start(const std::string& multicast_address, int multicast_port)
{
    _multicast_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(multicast_address), multicast_port);
    return Start();
}

inline bool UDPServer::Start(const asio::ip::udp::endpoint& multicast_endpoint)
{
    _multicast_endpoint = multicast_endpoint;
    return Start();
}

inline bool UDPServer::Stop()
{
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Update the started flag
        _started = false;

        // Close the server socket
        _socket.close();

        // Call the server stopped handler
        onStopped();
    });

    return true;
}

inline size_t UDPServer::Multicast(const void* buffer, size_t size)
{
    // Send the datagram to the multicast endpoint
    return Send(_multicast_endpoint, buffer, size);
}

inline size_t UDPServer::Send(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size)
{
    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Dispatch the send routine
    auto self(this->shared_from_this());
    service()->service().dispatch([this, self, endpoint, size]()
    {
        // Try to send the datagram
        TrySend(endpoint, size);
    });

    return _send_buffer.size();
}

inline void UDPServer::TryReceive()
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

        // Received datagram from the client
        if (received > 0)
        {
            // Prepare receive buffer
            _recive_buffer.resize(_recive_buffer.size() - (CHUNK - received));

            // Call the datagram received handler
            onReceived(_recive_endpoint, _recive_buffer.data(), _recive_buffer.size());

            // Clear the handled buffer
            _recive_buffer.clear();
        }

        // Check if the server is stopped
        if (!IsStarted())
            return;

        // Try to receive again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
            onError(ec.value(), ec.category().name(), ec.message());
    });
}

inline void UDPServer::TrySend(const asio::ip::udp::endpoint& endpoint, size_t size)
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    _socket.async_send_to(asio::const_buffer(_send_buffer.data(), _send_buffer.size()), endpoint, [this, self, endpoint](std::error_code ec, size_t sent)
    {
        _sending = false;

        // Sent datagram to the client
        if (sent > 0)
        {
            // Erase the sent buffer
            {
                std::lock_guard<std::mutex> locker(_send_lock);
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + sent);
            }

            // Call the datagram sent handler
            onSent(endpoint, sent, 0);

            // Stop sending
            return;
        }

        // Check if the server is stopped
        if (!IsStarted())
            return;

        // Try to send again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            return;
        else
            onError(ec.value(), ec.category().name(), ec.message());
    });
}

} // namespace Asio
} // namespace CppServer
