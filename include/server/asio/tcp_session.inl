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
const size_t TCPSession<TServer, TSession>::CHUNK;

template <class TServer, class TSession>
inline TCPSession<TServer, TSession>::TCPSession(std::shared_ptr<TCPServer<TServer, TSession>> server, asio::ip::tcp::socket&& socket)
    : _id(CppCommon::UUID::Generate()),
      _server(server),
      _socket(std::move(socket)),
      _connected(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _sending(false)
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
        _send_cache.insert(_send_cache.end(), bytes, bytes + size);
    }

    // Dispatch the send routine
    auto self(this->shared_from_this());
    service()->Dispatch([this, self]()
    {
        // Try to send the buffer
        TrySend();
    });

    return _send_cache.size();
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::TryReceive()
{
    if (_reciving)
        return;

    if (!IsConnected())
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_read_some(asio::buffer(_recive_buffer), [this, self](std::error_code ec, std::size_t size)
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
            _recive_cache.insert(_recive_cache.end(), _recive_buffer, _recive_buffer + size);

            // Call the buffer received handler
            size_t handled = onReceived(_recive_cache.data(), _recive_cache.size());

            // Erase the handled buffer
            _recive_cache.erase(_recive_cache.begin(), _recive_cache.begin() + handled);
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

    size_t size;
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the send buffer
        size = std::min(_send_cache.size(), CHUNK);
        std::memcpy(_send_buffer, _send_cache.data(), size);
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_socket, asio::buffer(_send_buffer, size), [this, self](std::error_code ec, std::size_t size)
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
            onSent(size, _send_cache.size());

            {
                std::lock_guard<std::mutex> locker(_send_lock);

                // Erase the sent buffer
                _send_cache.erase(_send_cache.begin(), _send_cache.begin() + size);

                // Stop sending if the send buffer is empty
                if (_send_cache.empty())
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

    _recive_cache.clear();
    _send_cache.clear();
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::SendError(std::error_code ec)
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
