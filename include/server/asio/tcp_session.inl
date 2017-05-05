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
      _sending(false),
      _recive_buffer(CHUNK),
      _send_buffer_main(CHUNK),
      _send_buffer_flush_offset(0)
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

    // Call the empty send buffer handler
    onEmpty();

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

    size_t result;
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the main send buffer
        const uint8_t* bytes = (const uint8_t*)buffer;
        _send_buffer_main.insert(_send_buffer_main.end(), bytes, bytes + size);
        result = _send_buffer_main.size();
    }

    // Dispatch the send routine
    auto self(this->shared_from_this());
    service()->Dispatch([this, self]()
    {
        // Try to send the main buffer
        TrySend();
    });

    return result;
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
    _socket.async_read_some(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), [this, self](std::error_code ec, std::size_t size)
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

            // Call the buffer received handler
            onReceived(_recive_buffer.data(), size);

            // If the receive buffer is full increase its size twice
            if (_recive_buffer.size() == size)
                _recive_buffer.resize(2 * size);
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

    // Swap send buffers
    if (_send_buffer_flush.empty())
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Swap flush and main buffers
        _send_buffer_flush.swap(_send_buffer_main);
        _send_buffer_flush_offset = 0;
    }

    // Check if the flush buffer is empty
    if (_send_buffer_flush.empty())
    {
        // Nothing to send...
        return;
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_socket, asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), [this, self](std::error_code ec, std::size_t size)
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
            onSent(size, _send_buffer_flush.size() - size);

            // Increase the flush buffer offset
            _send_buffer_flush_offset += size;

            // Successfully send the whole flush buffer
            if (_send_buffer_flush_offset == _send_buffer_flush.size())
            {
                // Clear the flush buffer
                _send_buffer_flush.clear();
                _send_buffer_flush_offset = 0;

                // Stop sending operation
                resume = false;
            }
        }

        // Try to send again if the session is valid
        if (!ec)
        {
            if (resume)
                TrySend();
            else
                onEmpty();
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
    // Clear send buffers
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        _send_buffer_main.clear();
        _send_buffer_flush.clear();
        _send_buffer_flush_offset = 0;
    }
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
