/*!
    \file server.inl
    \brief TCP server inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {

template <class TSession>
inline TCPServer::TCPServer(TCPProtocol protocol, uint16_t port) : _started(false)
{
    // Create TCP endpoint
    asio::tcp::endpoint endpoint;
    switch (protocol)
    {
        case IPv4:
            endpoint = tcp::endpoint(tcp::v4(), port);
            break;
        case IPv6:
            endpoint = tcp::endpoint(tcp::v6(), port);
            break;
    }

    // Create TCP acceptor
    _acceptor = tcp::acceptor(_service, endpoint);
}

template <class TSession>
inline TCPServer::TCPServer(const std::string& address, uint16_t port) : _started(false)
{
    // Create TCP resolver
    asio::tcp::resolver resolver(_service);

    // Create TCP resolver query
    asio::tcp::resolver::query query(address, port);

    // Create TCP endpoint
    asio::tcp::endpoint endpoint = tcp::endpoint(resolver.resolve(query));

    // Create TCP acceptor
    _acceptor = tcp::acceptor(_service, endpoint);
}

template <class TSession>
inline void TCPServer::Start()
{
    if (IsStarted())
        return;

    // Call server starting handler
    onStarting();

    // Start server thread
    _thread = std::thread([this]() { ServerLoop(); });

    // Update started flag
    _started = true;
}

template <class TSession>
inline void TCPServer::Stop()
{
    if (!IsStarted())
        return;

    // Call server stopping handler
    onStopping();

    // Update started flag
    _started = false;

    // Stop Asio service
    _service.stop();

    // Wait for server thread
    _thread.join();
}

template <class TSession>
inline void TCPServer::ServerAccept()
{
    _acceptor.async_accept(_socket, [this](std::error_code ec)
    {
        if (ec)
            onError(ec.value(), ec.category().name(), ec.message());
        else
        {
            // Create and register a new session
            auto session = std::make_shared<TSession>(*this, std::move(_socket));
            _sessions.emplace_back(session);
            onAccepted(session);
        }

        // Perform the next server accept
        ServerAccept();
    });
}

template <class TSession>
inline void TCPServer::ServerLoop()
{
    // Call initialize thread handler
    onThreadInitialize();

    try
    {
        // Call server started handler
        onStarted();

        // Perform the first server accept
        ServerAccept();

        // Run Asio service in a loop with skipping errors
        while (_started)
        {
            asio::error_code ec;
            _service.run(ec);
            if (ec)
                onError(ec.value(), ec.category().name(), ec.message());
        }

        // Clear sessions collection
        _sessions.clear();

        // Call server stopped handler
        onStopped();
    }
    catch (...)
    {
        fatality("TCP server thread terminated!");
    }

    // Call cleanup thread handler
    onThreadCleanup();
}

} // namespace CppServer
