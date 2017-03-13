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
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

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
inline TCPServer<TServer, TSession>::TCPServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _acceptor(_service->service()),
      _socket(_service->service()),
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    _endpoint = asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port);
}

template <class TServer, class TSession>
inline TCPServer<TServer, TSession>::TCPServer(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint)
    : _service(service),
      _endpoint(endpoint),
      _acceptor(_service->service()),
      _socket(_service->service()),
      _started(false),
      _bytes_sent(0),
      _bytes_received(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::Start()
{
    assert(!IsStarted() && "TCP server is already started!");
    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Create the server acceptor
        _acceptor = asio::ip::tcp::acceptor(_service->service(), _endpoint);

        // Reset statistic
        _bytes_sent = 0;
        _bytes_received = 0;

        // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Perform the first server accept
        Accept();
    });

    return true;
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::Stop()
{
    assert(IsStarted() && "TCP server is not started!");
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Close the server acceptor
        _acceptor.close();

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
inline bool TCPServer<TServer, TSession>::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::Accept()
{
    if (!IsStarted())
        return;

    // Dispatch the disconnect routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        _acceptor.async_accept(_socket, [this, self](std::error_code ec)
        {
            // Check if the server is stopped
            if (!IsStarted())
                return;

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
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return false;

    if (!IsStarted())
        return false;

    // Fill the multicast buffer
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);
        const uint8_t* bytes = (const uint8_t*)buffer;
        _multicast_buffer.insert(_multicast_buffer.end(), bytes, bytes + size);
    }

    // Dispatch the multicast routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        std::lock_guard<std::mutex> locker(_multicast_lock);

        // Check for empty multicast buffer
        if (_multicast_buffer.empty())
            return;

        // Multicast all sessions
        for (auto& session : _sessions)
            session.second->Send(_multicast_buffer.data(), _multicast_buffer.size());

        // Clear the multicast buffer
        _multicast_buffer.clear();
    });

    return true;
}

template <class TServer, class TSession>
inline bool TCPServer<TServer, TSession>::DisconnectAll()
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
inline std::shared_ptr<TSession> TCPServer<TServer, TSession>::RegisterSession()
{
    // Create and register a new session
    auto self(this->shared_from_this());
    auto session = std::make_shared<TSession>(self, std::move(_socket));
    _sessions.emplace(session->id(), session);

    // Connect a new session
    session->Connect();

    // Call a new session connected handler
    onConnected(session);

    return session;
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::UnregisterSession(const CppCommon::UUID& id)
{
    // Try to find the unregistered session
    auto it = _sessions.find(id);
    if (it != _sessions.end())
    {
        // Call the session disconnected handler
        onDisconnected(it->second);

        // Erase the session
        _sessions.erase(it);
    }
}

template <class TServer, class TSession>
inline void TCPServer<TServer, TSession>::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_multicast_lock);
    _multicast_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
