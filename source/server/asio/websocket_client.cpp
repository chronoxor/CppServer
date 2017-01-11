/*!
    \file websocket_client.cpp
    \brief WebSocket client implementation
    \author Ivan Shynkarenka
    \date 11.01.2016
    \copyright MIT License
*/

#include "server/asio/websocket_client.h"

namespace CppServer {
namespace Asio {

WebSocketClient::WebSocketClient(std::shared_ptr<Service> service, const std::string& uri)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _uri(uri),
      _initialized(false),
      _connected(false)
{
    InitAsio();
}

void WebSocketClient::InitAsio()
{
    if (_initialized)
        return;

    // Setup WebSocket client core Asio service
    websocketpp::lib::error_code ec;
    _core.init_asio(&_service->service(), ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return;
    }

    _initialized = true;
}

bool WebSocketClient::Connect()
{
    if (!_service->IsStarted())
        return false;

    if (!_initialized)
        return false;

    if (IsConnected())
        return false;

    // Post the connect routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        websocketpp::lib::error_code ec;

        // Setup WebSocket client core logging
        _core.set_access_channels(websocketpp::log::alevel::none);
        _core.set_error_channels(websocketpp::log::elevel::none);

        // Setup WebSocket server core handlers
        _core.set_open_handler([this](websocketpp::connection_hdl connection) { Connected(); });
        _core.set_close_handler([this](websocketpp::connection_hdl connection) { Disconnected(); });

        // Get the client connection
        _connection = _core.get_connection(_uri, ec);
        if (ec)
        {
            onError(ec.value(), ec.category().name(), ec.message());
            onDisconnected();
            return;
        }

        // Setup WebSocket client handlers
        _connection->set_message_handler([this](websocketpp::connection_hdl connection, WebSocketMessage message) { onReceived(message); });
        _connection->set_fail_handler([this](websocketpp::connection_hdl connection)
        {
            WebSocketServerCore::connection_ptr con = _core.get_con_from_hdl(connection);
            websocketpp::lib::error_code ec = con->get_ec();
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnected();
        });

        // Note that connect here only requests a connection. No network messages are
        // exchanged until the event loop starts running in the next line.
        _connection = _core.connect(_connection);
    });

    return true;
}

void WebSocketClient::Connected()
{
    // Update the connected flag
    _connected = true;

    // Call the client connected handler
    onConnected();
}

bool WebSocketClient::Disconnect(websocketpp::close::status::value code, const std::string& reason)
{
    if (!IsConnected())
        return false;

    // Close the client connection
    websocketpp::lib::error_code ec;
    _core.close(_connection, code, reason, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return false;
    }

    return true;
}

void WebSocketClient::Disconnected()
{
    // Update the connected flag
    _connected = false;

    // Call the client disconnected handler
    onDisconnected();
}

bool WebSocketClient::Reconnect()
{
    if (!Disconnect())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return Connect();
}

size_t WebSocketClient::Send(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode)
{
    if (!IsConnected())
        return 0;

    websocketpp::lib::error_code ec;
    _core.send(_connection, buffer, size, opcode, ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return 0;
    }

    return size;
}

} // namespace Asio
} // namespace CppServer
