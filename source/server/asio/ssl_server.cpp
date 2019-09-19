/*!
    \file ssl_server.cpp
    \brief SSL server implementation
    \author Ivan Shynkarenka
    \date 30.12.2016
    \copyright MIT License
*/

#include "server/asio/ssl_server.h"

namespace CppServer {
namespace Asio {

SSLServer::SSLServer(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, int port, InternetProtocol protocol)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _port(port),
      _context(context),
      _acceptor(*_io_service),
      _started(false),
      _bytes_pending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _option_keep_alive(false),
      _option_no_delay(false),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    assert((context != nullptr) && "SSL context is invalid!");
    if (context == nullptr)
        throw CppCommon::ArgumentException("SSL context is invalid!");

    // Prepare endpoint
    switch (protocol)
    {
        case InternetProtocol::IPv4:
            _endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), (unsigned short)port);
            break;
        case InternetProtocol::IPv6:
            _endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v6(), (unsigned short)port);
            break;
    }
}

SSLServer::SSLServer(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const std::string& address, int port)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _address(address),
      _port(port),
      _context(context),
      _acceptor(*_io_service),
      _started(false),
      _bytes_pending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _option_keep_alive(false),
      _option_no_delay(false),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    assert((context != nullptr) && "SSL context is invalid!");
    if (context == nullptr)
        throw CppCommon::ArgumentException("SSL context is invalid!");

    // Prepare endpoint
    _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(address), (unsigned short)port);
}

SSLServer::SSLServer(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const asio::ip::tcp::endpoint& endpoint)
    : _id(CppCommon::UUID::Sequential()),
      _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _address(endpoint.address().to_string()),
      _port(endpoint.port()),
      _context(context),
      _endpoint(endpoint),
      _acceptor(*_io_service),
      _started(false),
      _bytes_pending(0),
      _bytes_sent(0),
      _bytes_received(0),
      _option_keep_alive(false),
      _option_no_delay(false),
      _option_reuse_address(false),
      _option_reuse_port(false)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");

    assert((context != nullptr) && "SSL context is invalid!");
    if (context == nullptr)
        throw CppCommon::ArgumentException("SSL context is invalid!");
}

bool SSLServer::Start()
{
    assert(!IsStarted() && "SSL server is already started!");
    if (IsStarted())
        return false;

    // Post the start handler
    auto self(this->shared_from_this());
    auto start_handler = [this, self]()
    {
        if (IsStarted())
            return;

        // Create a server acceptor
        _acceptor = asio::ip::tcp::acceptor(*_io_service);
        _acceptor.open(_endpoint.protocol());
        if (option_reuse_address())
            _acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)
        if (option_reuse_port())
        {
            typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
            _acceptor.set_option(reuse_port(true));
        }
#endif
        _acceptor.bind(_endpoint);
        _acceptor.listen();

        // Reset statistic
        _bytes_pending = 0;
        _bytes_sent = 0;
        _bytes_received = 0;

        // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();

        // Perform the first server accept
        Accept();
    };
    if (_strand_required)
        _strand.post(start_handler);
    else
        _io_service->post(start_handler);

    return true;
}

bool SSLServer::Stop()
{
    assert(IsStarted() && "SSL server is not started!");
    if (!IsStarted())
        return false;

    // Post the stop handler
    auto self(this->shared_from_this());
    auto stop_handler = [this, self]()
    {
        if (!IsStarted())
            return;

        // Close the server acceptor
        _acceptor.close();

        // Reset the session
        _session->ResetServer();

        // Disconnect all sessions
        DisconnectAll();

        // Update the started flag
        _started = false;

        // Clear multicast buffer
        ClearBuffers();

        // Call the server stopped handler
        onStopped();
    };
    if (_strand_required)
        _strand.post(stop_handler);
    else
        _io_service->post(stop_handler);

    return true;
}

bool SSLServer::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

void SSLServer::Accept()
{
    if (!IsStarted())
        return;

    // Dispatch the accept handler
    auto self(this->shared_from_this());
    auto accept_handler = make_alloc_handler(_acceptor_storage, [this, self]()
    {
        if (!IsStarted())
            return;

        // Create a new session to accept
        _session = CreateSession(self);

        auto async_accept_handler = make_alloc_handler(_acceptor_storage, [this, self](std::error_code ec)
        {
            if (!ec)
            {
                RegisterSession();

                // Connect a new session
                _session->Connect();
            }
            else
                SendError(ec);

            // Perform the next server accept
            Accept();
        });
        if (_strand_required)
            _acceptor.async_accept(_session->socket(), bind_executor(_strand, async_accept_handler));
        else
            _acceptor.async_accept(_session->socket(), async_accept_handler);
    });
    if (_strand_required)
        _strand.dispatch(accept_handler);
    else
        _io_service->dispatch(accept_handler);
}

bool SSLServer::Multicast(const void* buffer, size_t size)
{
    if (!IsStarted())
        return false;

    if (size == 0)
        return true;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    std::shared_lock<std::shared_mutex> locker(_sessions_lock);

    // Multicast all sessions
    for (auto& session : _sessions)
        session.second->SendAsync(buffer, size);

    return true;
}

bool SSLServer::DisconnectAll()
{
    if (!IsStarted())
        return false;

    // Dispatch the disconnect all handler
    auto self(this->shared_from_this());
    auto disconnect_all_handler = [this, self]()
    {
        if (!IsStarted())
            return;

        std::shared_lock<std::shared_mutex> locker(_sessions_lock);

        // Disconnect all sessions
        for (auto& session : _sessions)
            session.second->Disconnect();
    };
    if (_strand_required)
        _strand.dispatch(disconnect_all_handler);
    else
        _io_service->dispatch(disconnect_all_handler);

    return true;
}

std::shared_ptr<SSLSession> SSLServer::FindSession(const CppCommon::UUID& id)
{
    std::shared_lock<std::shared_mutex> locker(_sessions_lock);

    // Try to find the required session
    auto it = _sessions.find(id);
    return (it != _sessions.end()) ? it->second : nullptr;
}

void SSLServer::RegisterSession()
{
    std::unique_lock<std::shared_mutex> locker(_sessions_lock);

    // Register a new session
    _sessions.emplace(_session->id(), _session);
}

void SSLServer::UnregisterSession(const CppCommon::UUID& id)
{
    std::unique_lock<std::shared_mutex> locker(_sessions_lock);

    // Try to find the unregistered session
    auto it = _sessions.find(id);
    if (it != _sessions.end())
    {
        // Erase the session
        _sessions.erase(it);
    }
}

void SSLServer::ClearBuffers()
{
    // Update statistic
    _bytes_pending = 0;
}

void SSLServer::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    // Skip Winsock error 995: The I/O operation has been aborted because of either a thread exit or an application request
    if (ec.value() == 995)
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
