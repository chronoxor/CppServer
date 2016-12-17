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
      _socket(_service->service())
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
      _socket(_service->service())
{
    // Create TCP endpoint
    asio::ip::tcp::endpoint endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port);

    // Create TCP acceptor
    _acceptor = asio::ip::tcp::acceptor(_service->service(), endpoint);
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::Accept()
{
    if (!_service->IsStarted())
        return;

    // Dispatch disconnect routine
    auto self(shared_from_this());
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
inline void TCPServer<TServer, TSession>::Broadcast(const void* buffer, size_t size)
{
    if (!_service->IsStarted())
        return;

    std::lock_guard<std::mutex> locker(_broadcast_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _broadcast_buffer.insert(_broadcast_buffer.end(), bytes, bytes + size);

    // Dispatch broadcast routine
    auto self(shared_from_this());
    _service->service().dispatch([this, self]()
    {
        // Broadcast all sessions
        for (auto& session : _sessions)
            session.second->Send(_broadcast_buffer.data(), _broadcast_buffer.size());

        // Clear broadcast buffer
        _broadcast_buffer.clear();
    });
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::DisconnectAll()
{
    if (!_service->IsStarted())
        return;

    // Post disconnect routine
    auto self(shared_from_this());
    _service->service().post([this, self]()
    {
        // Clear broadcast buffer
        {
            std::lock_guard<std::mutex> locker(_broadcast_lock);
            _broadcast_buffer.clear();
        }

        // Disconnect all sessions
        for (auto& session : _sessions)
            session.second->Disconnect();
    });
}

template <class TServer, class TSession>
inline std::shared_ptr<TSession> TCPServer<TServer, TSession>::RegisterSession()
{
	// Create and register a new session
    auto session = std::make_shared<TSession>(std::move(_socket));
    _sessions.emplace(session->id(), session);

    // Connect a new session
	auto self(shared_from_this());
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
