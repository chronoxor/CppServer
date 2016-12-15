/*!
    \file client.inl
    \brief TCP client inline implementation
    \author Ivan Shynkarenka
    \date 15.12.2016
    \copyright MIT License
*/

namespace CppServer {

template <class TServer, class TSession>
inline TCPSession<TServer, TSession>::TCPSession(TServer& server, const CppCommon::UUID& uuid, asio::ip::tcp::socket socket)
    : _server(server),
      _id(uuid),
      _socket(std::move(socket)),
      _disconnected(false)
{
    // Put the socket into non-blocking mode
    _socket.non_blocking(true);

    // Call session connected handler
    onConnected();

    // Try to receive something from the client
    TryReceive();
}

template <class TServer, class TSession>
void TCPSession<TServer, TSession>::Disconnect()
{
    if (IsConnected())
    {
        // Setup disconnected flag
        _disconnected = true;

        // Close the session on error
        _socket.close();

        // Call the session disconnected handler
        onDisconnected();

        // Unregister the session
        _server.UnregisterSession(id());
    }
}

template <class TServer, class TSession>
size_t TCPSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Try to send the buffer if it is the first buffer to send
    if ((size > 0) && (size == _send_buffer.size()))
        TrySend();

    return _send_buffer.size();
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::TryReceive()
{
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this](std::error_code ec)
    {
        // Perform receive some data from the client in non blocking mode
        if (!ec)
        {
            const size_t CHUNK = 8192;

            uint8_t buffer[CHUNK];
            size_t size = _socket.read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

                // Check for session disconnected state
                if (!IsConnected())
                    return;

                // Erase handled buffer
                _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
            }
        }

        // Try to receive again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
            Disconnect();
    });
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::TrySend()
{
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this](std::error_code ec)
    {
        // Perform send some data to the client in non blocking mode
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Call buffer sent handler
                onSent(size, _send_buffer.size());

                // Check for session disconnected state
                if (!IsConnected())
                    return;

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    return;
            }
        }

        // Try to send again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
            Disconnect();
    });
}

} // namespace CppServer
