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
inline TCPSession<TServer, TSession>::TCPSession(asio::ip::tcp::socket socket)
    : _id(CppCommon::UUID::Generate()),
      _socket(std::move(socket)),
      _connected(false),
      _reciving(false),
      _sending(false)
{
}

template <class TServer, class TSession>
inline void TCPSession<TServer, TSession>::Connect(std::shared_ptr<TCPServer<TServer, TSession>> server)
{
    // Assign the TCP server
    _server = server;

    // Put the socket into non-blocking mode
    _socket.non_blocking(true);

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();

    // Try to receive something from the client
    TryReceive();
}

template <class TServer, class TSession>
inline bool TCPSession<TServer, TSession>::Disconnect()
{
    if (!IsConnected())
        return false;

    // Post the disconnect routine
    auto self(this->shared_from_this());
    _server->service()->service().post([this, self]()
    {
        // Update the connected flag
        _connected = false;

        // Call the session disconnected handler
        onDisconnected();

        // Unregister the session
        _server->UnregisterSession(id());

        // Clear receive/send buffers
        _recive_buffer.clear();
        {
            std::lock_guard<std::mutex> locker(_send_lock);
            _send_buffer.clear();
        }

        // Close the session socket
        _socket.close();
    });

    return true;
}

template <class TServer, class TSession>
inline size_t TCPSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    if (!IsConnected())
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
inline void TCPSession<TServer, TSession>::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec)
    {
        _reciving = false;

        // Receive some data from the client in non blocking mode
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
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Send some data to the client in non blocking mode
        size_t sent = 0;
        size_t pending = 0;
        bool repeat = true;
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Fill sent handler parameters
                sent = size;
                pending = _send_buffer.size();

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    repeat = false;
            }
        }

        // Call the buffer sent handler
        if (sent > 0)
            onSent(sent, pending);

        // Stop the send loop if there is nothing to send
        if (!repeat)
            return;

        // Try to send again if the session is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
            Disconnect();
    });
}

} // namespace Asio
} // namespace CppServer
