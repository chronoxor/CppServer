/*!
    \file web_server.cpp
    \brief Web server implementation
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/asio/web_server.h"

namespace CppServer {
namespace Asio {

WebServer::WebServer(std::shared_ptr<Service> service, int port, bool ssl)
    : _service(service),
      _server(std::make_shared<restbed::Service>()),
      _settings(std::make_shared<restbed::Settings>()),
      _ssl_settings(std::make_shared<restbed::SSLSettings>()),
      _started(false),
      _ssl(ssl)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    // Use external IO service
    _server->set_external_io_service(_service->service());

    // Prepare Web server settings
    if (IsSSL())
    {
        _ssl_settings->set_port(port);
        _settings->set_ssl_settings(_ssl_settings);
    }
    else
    {
        _settings->set_port(port);
    }
}

WebServer::WebServer(std::shared_ptr<Service> service, const std::string& address, int port, bool ssl)
    : _service(service),
      _server(std::make_shared<restbed::Service>()),
      _settings(std::make_shared<restbed::Settings>()),
      _ssl_settings(std::make_shared<restbed::SSLSettings>()),
      _started(false),
      _ssl(ssl)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");

    // Use external IO service
    _server->set_external_io_service(_service->service());

    // Prepare Web server settings
    if (IsSSL())
    {
        _ssl_settings->set_bind_address(address);
        _ssl_settings->set_port(port);
        _settings->set_ssl_settings(_ssl_settings);
    }
    else
    {
        _settings->set_bind_address(address);
        _settings->set_port(port);
    }
}

bool WebServer::Start()
{
    assert(!IsStarted() && "Web server is already started!");
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

bool WebServer::Stop()
{
    assert(IsStarted() && "Web server is not started!");
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

bool WebServer::Restart()
{
    if (!Stop())
        return false;

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
}

} // namespace Asio
} // namespace CppServer
