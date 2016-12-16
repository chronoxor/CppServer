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
inline TCPSession<TServer, TSession>::TCPSession(TServer& server, const CppCommon::UUID& uuid, asio::ip::tcp::socket socket)
    : _id(uuid),
      _server(server),
      _socket(std::move(socket)),
      _connected(true),
      _reciving(false),
      _sending(false)
{
    // Put the socket into non-blocking mode
    _socket.non_blocking(true);

    // Post connect routine
    _server._service.service().post([this]()
    {
        // Call session connected handler
        onConnected();

        // Try to receive something from the client
        TryReceive();
    });
}

template <class TServer, class TSession>
bool TCPSession<TServer, TSession>::Disconnect()
{
    if (!IsConnected())
        return false;

    // Post disconnect routine
    _server._service.service().post([this]()
    {
        // Update connected flag
        _connected = false;

        // Clear receive/send buffers
        _recive_buffer.clear();
        {
            std::lock_guard<std::mutex> locker(_send_lock);
            _send_buffer.clear();
        }

        // Close the session socket
        _socket.close();

        // Call the session disconnected handler
        onDisconnected();

        // Unregister the session
        _server.UnregisterSession(id());
    });

    return true;
}

template <class TServer, class TSession>
size_t TCPSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    if (!IsConnected())
        return 0;

    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Post send routine
    _server._service.service().post([this]()
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
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this](std::error_code ec)
    {
        _reciving = false;

        // Perform receive some data from the client in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _socket.read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

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
    if (_sending)
        return;

    _sending = true;
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this](std::error_code ec)
    {
        _sending = false;

        // Perform send some data to the client in non blocking mode
        size_t sent = 0;
        size_t pending = 0;
        bool repeat = true;
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            std::error_code ec;
            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Fill sent handler parameters
                sent = size;
                pending = _send_buffer.size();

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    repeat = false;
            }
        }

        // Call buffer sent handler
        if (sent > 0)
            onSent(sent, pending);

        // Stop send loop if there is nothing to send
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
