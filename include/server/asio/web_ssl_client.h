/*!
    \file web_ssl_client.h
    \brief HTTPS Web client definition
    \author Ivan Shynkarenka
    \date 14.03.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEB_SSL_CLIENT_H
#define CPPSERVER_ASIO_WEB_SSL_CLIENT_H

#include "web_client.h"

namespace CppServer {
namespace Asio {

//! HTTPS Web client
/*!
    HTTPS Web client is used to send different kind of Web requests
    such as POST, GET, PUT, DELETE, etc. to any HTTPS Web server and
    receive responses in synchronous and asynchronous modes.

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class WebSSLClient : public WebClient
{
public:
    //! Initialize HTTPS Web server with a given Asio service
    /*!
        \param service - Asio service
    */
    explicit WebSSLClient(std::shared_ptr<Service> service);
    WebSSLClient(const WebSSLClient&) = delete;
    WebSSLClient(WebSSLClient&&) = default;
    virtual ~WebSSLClient() = default;

    WebSSLClient& operator=(const WebSSLClient&) = delete;
    WebSSLClient& operator=(WebSSLClient&&) = default;

    //! Get the Restbed SSL settings
    std::shared_ptr<restbed::SSLSettings>& ssl_settings() noexcept { return _ssl_settings; }

private:
    // Restbed SSL settings
    std::shared_ptr<restbed::SSLSettings> _ssl_settings;
};

/*! \example web_ssl_client_sync.cpp HTTPS Web synchronous client example */
/*! \example web_ssl_client_async.cpp HTTPS Web asynchronous client example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEB_SSL_CLIENT_H
