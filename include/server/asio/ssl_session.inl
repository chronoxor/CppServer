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
inline SSLSession<TServer, TSession>::SSLSession(asio::ip::tcp::socket&& socket, asio::ssl::context& context)
    : _id(CppCommon::UUID::Generate()),
      _stream(std::move(socket), context),
      _context(context),
      _connected(false),
      _handshaked(false),
      _reciving(false),
      _sending(false)
{
}

template <class TServer, class TSession>
inline void SSLSession<TServer, TSession>::Connect(std::shared_ptr<SSLServer<TServer, TSession>> server)
{
    // Assign the SSL server
    _server = server;

    // Put the socket into non-blocking mode
    socket().non_blocking(true);

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();

    // Perform SSL handshake
    auto self(this->shared_from_this());
    _stream.async_handshake(asio::ssl::stream_base::server, [this, self](std::error_code ec)
    {
        if (!ec)
        {
            // Update the handshaked flag
            _handshaked = true;

            // Call the session handshaked handler
            onHandshaked();

            // Try to receive something from the client
            TryReceive();
        }
        else
        {
            // Disconnect on in case of the bad handshake
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect(true);
        }
    });
}

template <class TServer, class TSession>
inline bool SSLSession<TServer, TSession>::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    // Post the disconnect routine
    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
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
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        _server->service()->service().dispatch(disconnect);
    else
        _server->service()->service().post(disconnect);

    return true;
}

template <class TServer, class TSession>
inline size_t SSLSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    if (!IsHandshaked())
        return 0;

    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Dispatch the send routine
    auto self(this->shared_from_this());
    _server->service()->service().dispatch([this, self]()
    {
        // Try to send the buffer if it is the first buffer to send
        if (!_sending)
            TrySend();
    });

    return _send_buffer.size();
}

template <class TServer, class TSession>
inline void SSLSession<TServer, TSession>::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    socket().async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec)
    {
        _reciving = false;

        // Receive some data from the client in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _stream.read_some(asio::buffer(buffer), ec);
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

        // Try to receive again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect(true);
        }
    });
}

template <class TServer, class TSession>
inline void SSLSession<TServer, TSession>::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    socket().async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Send some data to the client in non blocking mode
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _stream.write_some(asio::buffer(_send_buffer), ec);
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

        // Try to send again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect(true);
        }
    });
}

template <class TServer, class TSession>
inline void SSLSession<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_send_lock);
    _recive_buffer.clear();
    _send_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
