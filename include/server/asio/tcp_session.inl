/*!
    \file tcp_session.inl
    \brief TCP session inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
inline TCPSession<TServer, TSession>::TCPSession(std::shared_ptr<TCPServer<TServer, TSession>> server, asio::ip::tcp::socket&& socket)
    : _id(CppCommon::UUID::Generate()),
      _server(server),
      _socket(std::move(socket)),
      _connected(false),
      _reciving(false),
      _sending(false),
      _bytes_sent(0),
      _bytes_received(0)
{
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::Connect()
{
    // Reset statistic
    _bytes_sent = 0;
    _bytes_received = 0;

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();

    // Try to receive something from the client
    TryReceive();
}

template <class TServer, class TSession>
inline bool TCPSession<TServer, TSession>::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
        if (!IsConnected())
            return;

        // Shutdown the session socket
        _socket.shutdown(asio::ip::tcp::socket::shutdown_both);

        // Close the session socket
        _socket.close();

        // Clear receive/send buffers
        ClearBuffers();

        // Update the connected flag
        _connected = false;

        // Call the session disconnected handler
        onDisconnected();

        // Unregister the session
        _server->UnregisterSession(id());
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        service()->Dispatch(disconnect);
    else
        service()->Post(disconnect);

    return true;
}

template <class TServer, class TSession>
inline size_t TCPSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsConnected())
        return 0;

    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the send buffer
        const uint8_t* bytes = (const uint8_t*)buffer;
        _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);
    }

    // Dispatch the send routine
    auto self(this->shared_from_this());
    service()->Dispatch([this, self]()
    {
        // Try to send the buffer
        TrySend();
    });

    return _send_buffer.size();
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::TryReceive()
{
    if (_reciving)
        return;

    if (!IsConnected())
        return;

    uint8_t buffer[CHUNK];

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_read_some(asio::buffer(buffer), [this, self, &buffer](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsConnected())
            return;

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            _bytes_received += size;
            _server->_bytes_received += size;

            // Fill receive buffer
            _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

            // Call the buffer received handler
            size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

            // Erase the handled buffer
            _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
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

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::TrySend()
{
    if (_sending)
        return;

    if (!IsConnected())
        return;

    uint8_t buffer[CHUNK];
    size_t size;

    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the send buffer
        size = std::min(_send_buffer.size(), CHUNK);
        std::memcpy(buffer, _send_buffer.data(), size);
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_socket, asio::buffer(buffer, size), [this, self](std::error_code ec, std::size_t size)
    {
        _sending = false;

        if (!IsConnected())
            return;

        bool resume = true;

        // Send some data to the client
        if (size > 0)
        {
            // Update statistic
            _bytes_sent += size;
            _server->_bytes_sent += size;

            // Call the buffer sent handler
            onSent(size, _send_buffer.size());

            {
                std::lock_guard<std::mutex> locker(_send_lock);

                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    resume = false;
            }
        }

        // Try to send again if the session is valid
        if (!ec)
        {
            if (resume)
                TrySend();
        }
        else
        {
            SendError(ec);
            Disconnect(true);
        }
    });
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_send_lock);

    _recive_buffer.clear();
    _send_buffer.clear();
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof))
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
