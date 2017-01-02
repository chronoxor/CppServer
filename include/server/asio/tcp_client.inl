/*!
    \file tcp_client.inl
    \brief TCP client inline implementation
    \author Ivan Shynkarenka
    \date 15.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

inline TCPClient::TCPClient(std::shared_ptr<Service> service, const std::string& address, int port)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port)),
      _socket(_service->service()),
      _connected(false),
      _reciving(false),
      _sending(false)
{
}

inline TCPClient::TCPClient(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(endpoint),
      _socket(_service->service()),
      _connected(false),
      _reciving(false),
      _sending(false)
{
}

inline bool TCPClient::Connect()
{
    if (!_service->IsStarted())
        return false;

    if (IsConnected())
        return false;

    // Post the connect routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Connect the client socket
        _socket.async_connect(_endpoint, [this, self](std::error_code ec)
        {
            if (!ec)
            {
                // Put the socket into non-blocking mode
                _socket.non_blocking(true);

                // Set the socket keep-alive option
                asio::ip::tcp::socket::keep_alive keep_alive(true);
                _socket.set_option(keep_alive);

                // Update the connected flag
                _connected = true;

                // Call the client connected handler
                onConnected();

                // Try to receive something from the server
                TryReceive();
            }
            else
            {
                // Call the client disconnected handler
                onError(ec.value(), ec.category().name(), ec.message());
                onDisconnected();
            }
        });
    });

    return true;
}

inline bool TCPClient::Disconnect()
{
    if (!IsConnected())
        return false;

    // Post the disconnect routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Update the connected flag
        _connected = false;

        // Clear receive/send buffers
        ClearBuffers();

        // Close the client socket
        _socket.close();

        // Call the client disconnected handler
        onDisconnected();
    });

    return true;
}

inline size_t TCPClient::Send(const void* buffer, size_t size)
{
    if (!IsConnected())
        return 0;

    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Dispatch the send routine
    auto self(this->shared_from_this());
    _service->service().dispatch([this, self]()
    {
        // Try to send the buffer if it is the first buffer to send
        if (!_sending)
            TrySend();
    });

    return _send_buffer.size();
}

inline void TCPClient::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec)
    {
        _reciving = false;

        // Receive some data from the server in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _socket.read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call the buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

                // Erase the handled buffer
                _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
            }
        }

        // Check for disconnect
        if (!IsConnected())
            return;

        // Try to receive again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect();
        }
    });
}

inline void TCPClient::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Send some data to the server in non blocking mode
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Call the buffer sent handler
                onSent(size, _send_buffer.size());

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    return;
            }
        }

        // Check for disconnect
        if (!IsConnected())
            return;

        // Try to send again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect();
        }
    });
}

inline void TCPClient::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_send_lock);
    _recive_buffer.clear();
    _send_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
