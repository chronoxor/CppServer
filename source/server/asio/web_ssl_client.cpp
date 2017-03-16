/*!
    \file web_ssl_client.cpp
    \brief HTTPS Web client implementation
    \author Ivan Shynkarenka
    \date 14.03.2017
    \copyright MIT License
*/

#include "server/asio/web_ssl_client.h"

namespace CppServer {
namespace Asio {

WebSSLClient::WebSSLClient(std::shared_ptr<Service> service)
    : WebClient(service),
      _ssl_settings(std::make_shared<restbed::SSLSettings>())
{
    // Prepare SSL settings
    settings()->set_ssl_settings(_ssl_settings);
}

} // namespace Asio
} // namespace CppServer
