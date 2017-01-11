/*!
    \file websocket_server.inl
    \brief WebSocket server inline implementation
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
inline WebSocketServer<TServer, TSession>::WebSocketServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port)
    : _service(service),
      _started(false)
{
    switch (protocol)
    {
        case InternetProtocol::IPv4:
            _endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
            break;
        case InternetProtocol::IPv6:
            _endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port);
            break;
    }
}

template <class TServer, class TSession>
inline WebSocketServer<TServer, TSession>::WebSocketServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _started(false)
{
    _endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port);
}

template <class TServer, class TSession>
inline WebSocketServer<TServer, TSession>::WebSocketServer(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _started(false)
{
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Start()
{
    if (!_service->IsStarted())
        return false;

    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        websocketpp::lib::error_code ec;

        // Setup WebSocket server core logging
        _core.set_access_channels(websocketpp::log::alevel::all);
        _core.set_error_channels(websocketpp::log::elevel::all);

        // Setup WebSocket server core Asio service
        _core.init_asio(&_service->service(), ec);
        if (ec)
        {
            onError(ec.value(), ec.category().name(), ec.message());
            return;
        }

        // Setup WebSocket server core handlers
        _core.set_open_handler([this](websocketpp::connection_hdl connection) { RegisterSession(connection); });
        _core.set_close_handler([this](websocketpp::connection_hdl connection) { UnregisterSession(connection); });

        // Start WebSocket server core
        _core.listen(_endpoint, ec);
        if (ec)
        {
            onError(ec.value(), ec.category().name(), ec.message());
            return;
        }

        // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Start WebSocket core acceptor
        _core.start_accept(ec);
        if (ec)
            onError(ec.value(), ec.category().name(), ec.message());
    });

    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Stop()
{
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Disconnect all sessions
        DisconnectAll();

        // Stop WebSocket server
        websocketpp::lib::error_code ec;
        _core.stop_listening(ec);
        if (ec)
            onError(ec.value(), ec.category().name(), ec.message());

        // Clear multicast buffer
        ClearBuffers();

        // Update the started flag
        _started = false;

        // Call the server stopped handler
        onStopped();
    });

    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Multicast(const void* buffer, size_t size, websocketpp::frame::opcode::value opcode)
{
    if (!IsStarted())
        return false;

    std::lock_guard<std::mutex> locker(_multicast_lock);

    std::vector<uint8_t> message((const uint8_t*)buffer, ((const uint8_t*)buffer) + size);
    _multicast_buffer.emplace_back(std::make_tuple(message, opcode));

    // Dispatch the multicast routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        // Multicast all sessions
        for (auto& session : _sessions)
            for (auto& message : _multicast_buffer)
                session.second->Send(std::get<0>(message).data(), std::get<0>(message).size(), std::get<1>(message));

        // Clear the multicast buffer
        _multicast_buffer.clear();
    });

    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::DisconnectAll()
{
    if (!IsStarted())
        return false;

    // Dispatch the disconnect routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        // Disconnect all sessions
        for (auto& session : _sessions)
            session.second->Disconnect();
    });

    return true;
}

template <class TServer, class TSession>
inline std::shared_ptr<TSession> WebSocketServer<TServer, TSession>::RegisterSession(websocketpp::connection_hdl connection)
{
    // Create and register a new session
    auto self(this->shared_from_this());
    auto session = std::make_shared<TSession>(self);
    _connections.insert(std::make_pair(connection, session));
    _sessions.insert(std::make_pair(session->id(), session));

    // Connect a new session
    session->Connect(connection);

    // Call a new session connected handler
    onConnected(session);

    return session;
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::UnregisterSession(websocketpp::connection_hdl connection)
{
    // Try to find the unregistered connection
    auto it = _connections.find(connection);
    if (it != _connections.end())
    {
        // Call the session disconnected handler
        onDisconnected(it->second);

        // Erase the connection
        _connections.erase(it);

        // Erase the session
        _sessions.erase(_sessions.find(it->second->id()));
    }
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::UnregisterSession(const CppCommon::UUID& id)
{
    // Try to find the unregistered session
    auto it = _sessions.find(id);
    if (it != _sessions.end())
    {
        // Call the session disconnected handler
        onDisconnected(it->second);

        // Erase the session
        _sessions.erase(it);

        // Erase the connection
        _connections.erase(_connections.find(it->second->connection()));
    }
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_multicast_lock);
    _multicast_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
