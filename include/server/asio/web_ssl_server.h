/*!
    \file web_ssl_server.h
    \brief HTTPS Web server definition
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEB_SSL_SERVER_H
#define CPPSERVER_ASIO_WEB_SSL_SERVER_H

#include "web_server.h"

namespace CppServer {
namespace Asio {

//! HTTPS Web server
/*!
    HTTPS Web server is used to provide HTTPS interface to handle different
    kind of Web requests such as POST, GET, PUT, DELETE, etc.

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class WebSSLServer : public WebServer
{
public:
    //! Initialize HTTPS Web server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
    */
    explicit WebSSLServer(std::shared_ptr<Service> service, int port);
    //! Initialize HTTPS Web server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit WebSSLServer(std::shared_ptr<Service> service, const std::string& address, int port);
    WebSSLServer(const WebSSLServer&) = delete;
    WebSSLServer(WebSSLServer&&) noexcept = default;
    virtual ~WebSSLServer() = default;

    WebSSLServer& operator=(const WebSSLServer&) = delete;
    WebSSLServer& operator=(WebSSLServer&&) noexcept = default;

    //! Get the Restbed SSL settings
    std::shared_ptr<restbed::SSLSettings>& ssl_settings() noexcept { return _ssl_settings; }

private:
    // Restbed SSL settings
    std::shared_ptr<restbed::SSLSettings> _ssl_settings;
};

/*! \example web_ssl_server.cpp HTTPS Web server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEB_SSL_SERVER_H
