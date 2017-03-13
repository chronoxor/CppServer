/*!
    \file rest_server.cpp
    \brief REST server implementation
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/asio/rest_server.h"

namespace CppServer {
namespace Asio {

RestServer::RestServer(std::shared_ptr<Service> service, int port)
    : _service(service),
      _server(std::make_shared<restbed::Service>()),
      _settings(std::make_shared<restbed::Settings>()),
      _ssl_settings(std::make_shared<restbed::SSLSettings>()),
      _started(false)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    // Use external IO service
    _server->set_external_io_service(_service->service());

    // Prepare settings
    _settings->set_port(port);

    // Prepare SSL settings
    _ssl_settings->set_port(port);
}

RestServer::RestServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : _service(service),
      _server(std::make_shared<restbed::Service>()),
      _settings(std::make_shared<restbed::Settings>()),
      _ssl_settings(std::make_shared<restbed::SSLSettings>()),
      _started(false)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    // Use external IO service
    _server->set_external_io_service(_service->service());

    // Prepare settings
    _settings->set_bind_address(address);
    _settings->set_port(port);

    // Prepare SSL settings
    _ssl_settings->set_bind_address(address);
    _ssl_settings->set_port(port);
}

bool RestServer::Start()
{
    assert(!IsStarted() && "REST server is already started!");
    if (IsStarted())
        return false;

    // Post the start routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        // Start the Restbed server
        _server->start(_settings);

        // Update the started flag
        _started = true;

        // Call the server started handler
        onStarted();
    });

    return true;
}

bool RestServer::Stop()
{
    assert(IsStarted() && "REST server is not started!");
    if (!IsStarted())
        return false;

    // Post the stopped routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        // Stop the Restbed server
        _server->stop();

        // Update the started flag
        _started = false;

        // Call the server stopped handler
        onStopped();
    });

    return true;
}

bool RestServer::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

} // namespace Asio
} // namespace CppServer
