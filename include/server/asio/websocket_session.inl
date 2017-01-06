/*!
    \file websocket_session.inl
    \brief WebSocket session inline implementation
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
inline WebSocketSession<TServer, TSession>::WebSocketSession(std::shared_ptr<WebSocketServer<TServer, TSession>> server)
    : _id(CppCommon::UUID::Generate()),
      _server(server),
      _connected(false),
      _reciving(false),
      _sending(false)
{
}

template <class TServer, class TSession>
inline void WebSocketSession<TServer, TSession>::Connect(websocketpp::connection_hdl connection)
{
    // Assign new WebSocket connection
    _connection = connection;

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();
}

template <class TServer, class TSession>
inline bool WebSocketSession<TServer, TSession>::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
        // Close the session socket
        _server->close(_connection, 0);

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
inline size_t WebSocketSession<TServer, TSession>::Send(const void* buffer, size_t size)
{
    if (!IsConnected())
        return 0;

    std::lock_guard<std::mutex> locker(_send_lock);

    wsconnection::message_ptr message(buffer, size, binary);
    _send_buffer.emplace_back(message);

    // Dispatch the send routine
    auto self(this->shared_from_this());
    service()->Dispatch([this, self]()
    {
        // Try to send the buffer if it is the first buffer to send
        if (!_sending)
            TrySend();
    });

    return _send_buffer.size();
}

template <class TServer, class TSession>
inline void WebSocketSession<TServer, TSession>::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Send some data to the client in non blocking mode
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
inline void WebSocketSession<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_send_lock);
    _recive_buffer.clear();
    _send_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
