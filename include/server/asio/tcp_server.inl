/*!
    \file tcp_server.inl
    \brief TCP server inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TServer, class TSession>
inline TCPServer<TServer, TSession>::TCPServer(std::shared_ptr<Service> service, InternetProtocol protocol, int port)
    : _service(service),
      _acceptor(_service->service()),
      _socket(_service->service()),
      _started(false)
{
    // Create TCP endpoint
    asio::ip::tcp::endpoint endpoint;
    switch (protocol)
    {
        case InternetProtocol::IPv4:
            endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);
            break;
        case InternetProtocol::IPv6:
            endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v6(), port);
            break;
    }

    // Create TCP acceptor
    _acceptor = asio::ip::tcp::acceptor(_service->service(), endpoint);
}

template <class TServer, class TSession>
inline TCPServer<TServer, TSession>::TCPServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _acceptor(_service->service()),
      _socket(_service->service()),
      _started(false)
{
    // Create TCP endpoint
    asio::ip::tcp::endpoint endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port);

    // Create TCP acceptor
    _acceptor = asio::ip::tcp::acceptor(_service->service(), endpoint);
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::Start()
{
    if (!_service->IsStarted())
        return false;

    if (IsStarted())
        return false;

    // Post start routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
         // Update started flag
        _started = true;

        // Call server started handler
        onStarted();

        // Perform the first server accept
        Accept();
    });

    return true;
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::Stop()
{
    if (!IsStarted())
        return false;

    // Post stopped routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Disconnect all sessions
        DisconnectAll();

        // Update started flag
        _started = false;

        // Call server stopped handler
        onStopped();
    });

    return true;
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::Accept()
{
    if (!IsStarted())
        return;

    // Dispatch disconnect routine
    auto self(this->shared_from_this());
    _service->service().dispatch([this, self]()
    {
        _acceptor.async_accept(_socket, [this, self](std::error_code ec)
        {
            if (!ec)
                RegisterSession();
            else
                onError(ec.value(), ec.category().name(), ec.message());

            // Perform the next server accept
            Accept();
        });
    });
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::Multicast(const void* buffer, size_t size)
{
    if (!IsStarted())
        return false;

    std::lock_guard<std::mutex> locker(_multicast_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _multicast_buffer.insert(_multicast_buffer.end(), bytes, bytes + size);

    // Dispatch multicast routine
    auto self(this->shared_from_this());
    _service->service().dispatch([this, self]()
    {
        // Multicast all sessions
        for (auto& session : _sessions)
            session.second->Send(_multicast_buffer.data(), _multicast_buffer.size());

        // Clear multicast buffer
        _multicast_buffer.clear();
    });

    return true;
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::DisconnectAll()
{
    if (!IsStarted())
        return false;

    // Dispatch disconnect routine
    auto self(this->shared_from_this());
    _service->service().dispatch([this, self]()
    {
        // Clear multicast buffer
        {
            std::lock_guard<std::mutex> locker(_multicast_lock);
            _multicast_buffer.clear();
        }

        // Disconnect all sessions
        for (auto& session : _sessions)
            session.second->Disconnect();
    });

    return true;
}

template <class TServer, class TSession>
inline std::shared_ptr<TSession> TCPServer<TServer, TSession>::RegisterSession()
{
    // Create and register a new session
    auto session = std::make_shared<TSession>(std::move(_socket));
    _sessions.emplace(session->id(), session);

    // Connect a new session
    auto self(this->shared_from_this());
    session->Connect(self);

    // Call a new session connected handler
    onConnected(session);

    return session;
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::UnregisterSession(const CppCommon::UUID& id)
{
    // Try to find session to unregister
    auto it = _sessions.find(id);
    if (it != _sessions.end())
    {
        // Call the session disconnected handler
        onDisconnected(it->second);

        // Erase the session
        _sessions.erase(it);
    }
}

} // namespace Asio
} // namespace CppServer
