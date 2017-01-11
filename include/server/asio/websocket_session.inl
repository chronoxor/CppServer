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
      _connected(false)
{
}

template <class TServer, class TSession>
inline void WebSocketSession<TServer, TSession>::Connect(websocketpp::connection_hdl connection)
{
    // Setup WebSocket session handlers
    WebSocketServerCore::connection_ptr con = _server->core().get_con_from_hdl(connection);
    con->set_message_handler([this](websocketpp::connection_hdl connection, WebSocketMessage message) { onReceived(message); });
    con->set_fail_handler([this](websocketpp::connection_hdl connection)
    {
        WebSocketServerCore::connection_ptr con = _server->core().get_con_from_hdl(connection);
        websocketpp::lib::error_code ec = con->get_ec();
        onError(ec.value(), ec.category().name(), ec.message());
    });

    // Assign new WebSocket connection
    _connection = connection;

    // Update the connected flag
    _connected = true;

    // Call the session connected handler
    onConnected();
}

template <class TServer, class TSession>
inline bool WebSocketSession<TServer, TSession>::Disconnect(websocketpp::close::status::value code, const std::string& reason)
{
    if (!IsConnected())
        return false;

    // Close the session connection
    websocketpp::lib::error_code ec;
    _server->core().close(_connection, code, reason, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return false;
    }

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
    if (!IsConnected())
        return 0;

    websocketpp::lib::error_code ec;
    _server->core().send(_connection, buffer, size, opcode, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return 0;
    }

    return size;
}

} // namespace Asio
} // namespace CppServer
