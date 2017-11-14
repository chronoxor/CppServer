/*!
    \file ssl_session.inl
    \brief SSL session inline implementation
    \author Ivan Shynkarenka
    \date 30.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
inline SSLSession<TServer, TSession>::SSLSession(std::shared_ptr<SSLServer<TServer, TSession>> server, asio::ip::tcp::socket&& socket, std::shared_ptr<asio::ssl::context> context)
    : _id(CppCommon::UUID::Generate()),
      _server(server),
      _stream(std::move(socket), *context),
      _context(context),
      _connected(false),
      _handshaked(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _sending(false),
      _recive_buffer(CHUNK + 1),
      _send_buffer_flush_offset(0)
{
}

template <class TServer, class TSession>
inline void SSLSession<TServer, TSession>::Connect()
{
    if (IsConnected() || IsHandshaked())
        return;

    // Reset statistic
    _bytes_sent = 0;
    _bytes_received = 0;

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();

    // Perform SSL handshake
    auto self(this->shared_from_this());
    _stream.async_handshake(asio::ssl::stream_base::server, [this, self](std::error_code ec)
    {
        if (IsHandshaked())
            return;

        if (!ec)
        {
            // Update the handshaked flag
            _handshaked = true;

            // Call the session handshaked handler
            onHandshaked();

            // Call the empty send buffer handler
            onEmpty();

            // Try to receive something from the client
            TryReceive();
        }
        else
        {
            // Disconnect on in case of the bad handshake
            SendError(ec);
            Disconnect(true);
        }
    });
}

template <class TServer, class TSession>
inline bool SSLSession<TServer, TSession>::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
        if (!IsConnected())
            return;

        // Shutdown the client stream
        _stream.async_shutdown([this, self](std::error_code ec)
        {
            if (!IsConnected())
                return;

            // Close the session socket
            socket().close();

            // Clear receive/send buffers
            ClearBuffers();

            // Update the handshaked flag
            _handshaked = false;

            // Update the connected flag
            _connected = false;

            // Call the session disconnected handler
            onDisconnected();

            // Unregister the session
            _server->UnregisterSession(id());
        });
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        service()->Dispatch(disconnect);
    else
        service()->Post(disconnect);

    return true;
}

template <class TServer, class TSession>
inline size_t SSLSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsHandshaked())
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
inline void SSLSession<TServer, TSession>::TryReceive()
{
    if (_reciving)
        return;

    if (!IsHandshaked())
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _stream.async_read_some(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), [this, self](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsHandshaked())
            return;

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            _bytes_received += size;
            _server->_bytes_received += size;

            // If the receive buffer is full increase its size
            if (_recive_buffer.size() == size)
                _recive_buffer.resize(2 * size);

            // Call the buffer received handler
            onReceived(_recive_buffer.data(), size);
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
inline void SSLSession<TServer, TSession>::TrySend()
{
    if (_sending)
        return;

    if (!IsHandshaked())
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
        // Call the empty send buffer handler
        onEmpty();
        return;
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_stream, asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), [this, self](std::error_code ec, std::size_t size)
    {
        _sending = false;

        if (!IsHandshaked())
            return;

        // Send some data to the client
        if (size > 0)
        {
            // Update statistic
            _bytes_sent += size;
            _server->_bytes_sent += size;

            // Increase the flush buffer offset
            _send_buffer_flush_offset += size;

            // Successfully send the whole flush buffer
            if (_send_buffer_flush_offset == _send_buffer_flush.size())
            {
                // Clear the flush buffer
                _send_buffer_flush.clear();
                _send_buffer_flush_offset = 0;
            }

            // Call the buffer sent handler
            onSent(size, _send_buffer_flush.size() - _send_buffer_flush_offset);
        }

        // Try to send again if the session is valid
        if (!ec)
        {
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
inline void SSLSession<TServer, TSession>::ClearBuffers()
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
inline void SSLSession<TServer, TSession>::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
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
