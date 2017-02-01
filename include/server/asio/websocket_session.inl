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
      _messages_sent(0),
      _messages_received(0),
      _bytes_sent(0),
      _bytes_received(0)
{
}

template <class TServer, class TSession>
inline void WebSocketSession<TServer, TSession>::Connect(websocketpp::connection_hdl connection)
{
    // Setup WebSocket session handlers
    WebSocketServerCore::connection_ptr con = _server->core().get_con_from_hdl(connection);
    con->set_message_handler([this](websocketpp::connection_hdl connection, WebSocketMessage message)
    {
        size_t size = message->get_raw_payload().size();

        // Update statistic
        ++_messages_received;
        ++server()->_messages_received;
        _bytes_received += size;
        server()->_bytes_received += size;

        // Call the message received handler
        onReceived(message);
    });
    con->set_fail_handler([this](websocketpp::connection_hdl connection)
    {
        WebSocketServerCore::connection_ptr con = _server->core().get_con_from_hdl(connection);
        websocketpp::lib::error_code ec = con->get_ec();
        onError(ec.value(), ec.category().name(), ec.message());
        Disconnected();
    });

    // Assign new WebSocket connection
    _connection = connection;

    // Reset statistic
    _messages_sent = 0;
    _messages_received = 0;
    _bytes_sent = 0;
    _bytes_received = 0;

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();
}

template <class TServer, class TSession>
inline bool WebSocketSession<TServer, TSession>::Disconnect(bool dispatch, websocketpp::close::status::value code, const std::string& reason)
{
    if (!IsConnected())
        return false;

    auto self(this->shared_from_this());
    auto disconnect = [this, self, code, reason]()
    {
        // Close the session connection
        websocketpp::lib::error_code ec;
        _server->core().close(_connection, code, reason, ec);
        if (ec)
            onError(ec.value(), ec.category().name(), ec.message());
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        service()->Dispatch(disconnect);
    else
        service()->Post(disconnect);

    return true;
}

template <class TServer, class TSession>
inline void WebSocketSession<TServer, TSession>::Disconnected()
{
    // Update the connected flag
    _connected = false;

    // Call the session disconnected handler
    onDisconnected();

    // Unregister the session
    _server->UnregisterSession(id());
}

template <class TServer, class TSession>
inline size_t WebSocketSession<TServer, TSession>::Send(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsConnected())
        return 0;

    websocketpp::lib::error_code ec;
    _server->core().send(_connection, buffer, size, opcode, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return 0;
    }

    // Update statistic
    ++_messages_sent;
    ++server()->_messages_sent;
    _bytes_sent += size;
    server()->_bytes_sent += size;

    return size;
}

template <class TServer, class TSession>
inline size_t WebSocketSession<TServer, TSession>::Send(const std::string& text, websocketpp::frame::opcode::value opcode)
{
    if (!IsConnected())
        return 0;

    websocketpp::lib::error_code ec;
    _server->core().send(_connection, text, opcode, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return 0;
    }

    size_t size = text.size();

    // Update statistic
    ++_messages_sent;
    ++server()->_messages_sent;
    _bytes_sent += size;
    server()->_bytes_sent += size;

    return size;
}

template <class TServer, class TSession>
inline size_t WebSocketSession<TServer, TSession>::Send(WebSocketMessage message)
{
    if (!IsConnected())
        return 0;

    websocketpp::lib::error_code ec;
    _server->core().send(_connection, message, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return 0;
    }

    size_t size = message->get_raw_payload().size();

    // Update statistic
    ++_messages_sent;
    ++server()->_messages_sent;
    _bytes_sent += size;
    server()->_bytes_sent += size;

    return size;
}

} // namespace Asio
} // namespace CppServer
