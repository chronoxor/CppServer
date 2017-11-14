/*!
    \file web_ssl_server.cpp
    \brief HTTPS Web server implementation
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/asio/web_ssl_server.h"

namespace CppServer {
namespace Asio {

WebSSLServer::WebSSLServer(std::shared_ptr<Service> service, int port)
    : WebServer(service, port),
      _ssl_settings(std::make_shared<restbed::SSLSettings>())
{
    // Prepare SSL settings
    _ssl_settings->set_port(port);
    settings()->set_ssl_settings(_ssl_settings);
}

WebSSLServer::WebSSLServer(std::shared_ptr<Service> service, const std::string& address, int port)
    : WebServer(service, address, port),
      _ssl_settings(std::make_shared<restbed::SSLSettings>())
{
    // Prepare SSL settings
    _ssl_settings->set_bind_address(address);
    _ssl_settings->set_port(port);
    settings()->set_ssl_settings(_ssl_settings);
}

} // namespace Asio
} // namespace CppServer
