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
      _initialized(false),
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
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

    InitAsio();
}

template <class TServer, class TSession>
inline WebSocketServer<TServer, TSession>::WebSocketServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _initialized(false),
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
{
    _endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port);

    InitAsio();
}

template <class TServer, class TSession>
inline WebSocketServer<TServer, TSession>::WebSocketServer(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _initialized(false),
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
{
    InitAsio();
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::InitAsio()
{
    assert(!_initialized && "Asio is already initialed!");
    if (_initialized)
        return;

    // Setup WebSocket server core Asio service
    websocketpp::lib::error_code ec;
    _core.init_asio(&_service->service(), ec);
    if (ec)
    {
        onError(ec.value(), ec.category().name(), ec.message());
        return;
    }

    _initialized = true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Start()
{
    assert(_initialized && "Asio is not initialed!");
    if (!_initialized)
        return false;

    assert(!IsStarted() && "WebSocket server is already started!");
    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        websocketpp::lib::error_code ec;

        // Setup WebSocket server core logging
        _core.set_access_channels(websocketpp::log::alevel::none);
        _core.set_error_channels(websocketpp::log::elevel::none);

        // Setup WebSocket server core handlers
        _core.set_open_handler([this](websocketpp::connection_hdl connection) { RegisterSession(connection); });
        _core.set_close_handler([this](websocketpp::connection_hdl connection) { UnregisterSession(connection); });

        // Start WebSocket server core
        _core.listen(_endpoint, ec);
        if (ec)
        {
            onError(ec.value(), ec.category().name(), ec.message());
            onStopped();
            return;
        }

        // Reset statistic
        _bytes_sent = 0;
        _bytes_received = 0;

        // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Start WebSocket core acceptor
        _core.start_accept(ec);
        if (ec)
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Stop();
            return;
        }
    });

    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Stop()
{
    assert(IsStarted() && "WebSocket server is not started!");
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Stop WebSocket server
        websocketpp::lib::error_code ec;
        _core.stop_listening(ec);
        if (ec)
            onError(ec.value(), ec.category().name(), ec.message());

        // Clear multicast buffer
        ClearBuffers();

        // Disconnect all sessions
        DisconnectAll();

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
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return false;

    if (!IsStarted())
        return false;

    // Fill the multicast buffer
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);
        std::vector<uint8_t> message((const uint8_t*)buffer, ((const uint8_t*)buffer) + size);
        _multicast_buffer.emplace_back(std::make_tuple(message, opcode));
    }

    MulticastAll();
    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Multicast(const std::string& text, websocketpp::frame::opcode::value opcode)
{
    if (!IsStarted())
        return false;

    // Fill the multicast buffer
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);
        _multicast_text.emplace_back(std::make_tuple(text, opcode));
    }

    MulticastAll();
    return true;
}

template <class TServer, class TSession>
inline bool WebSocketServer<TServer, TSession>::Multicast(WebSocketMessage message)
{
    if (!IsStarted())
        return false;

    // Fill the multicast buffer
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);
        _multicast_messages.push_back(message);
    }

    MulticastAll();
    return true;
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::MulticastAll()
{
    // Dispatch the multicast routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);

        // Multicast all sessions
        for (auto& session : _sessions)
        {
            for (auto& message : _multicast_buffer)
                session.second->Send(std::get<0>(message).data(), std::get<0>(message).size(), std::get<1>(message));
            for (auto& text : _multicast_text)
                session.second->Send(std::get<0>(text), std::get<1>(text));
            for (auto& message : _multicast_messages)
                session.second->Send(message);
        }

        // Clear the multicast buffers
        _multicast_buffer.clear();
        _multicast_text.clear();
        _multicast_messages.clear();
    });
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
        it->second->Disconnected();
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

        // Erase the connection
        _connections.erase(_connections.find(it->second->connection()));

        // Erase the session
        _sessions.erase(it);
    }
}

template <class TServer, class TSession>
inline void WebSocketServer<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_multicast_lock);
    _multicast_buffer.clear();
    _multicast_text.clear();
    _multicast_messages.clear();
}

} // namespace Asio
} // namespace CppServer
