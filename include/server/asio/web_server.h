/*!
    \file web_server.h
    \brief Web server definition
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_WEB_SERVER_H
#define CPPSERVER_ASIO_WEB_SERVER_H

#include "asio.h"
#include "service.h"
#include "web.h"

namespace CppServer {
namespace Asio {

//! Web server
/*!
    Web server is used to provide HTTP/HTTPS interface to handle different kind
    of Web requests such as POST, GET, PUT, DELETE, etc.

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class WebServer : public std::enable_shared_from_this<WebServer>
{
public:
    //! Initialize Web server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
        \param ssl - SSL flag (default is false)
    */
    explicit WebServer(std::shared_ptr<Service> service, int port, bool ssl = false);
    //! Initialize Web server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
        \param ssl - SSL flag (default is false)
    */
    explicit WebServer(std::shared_ptr<Service> service, const std::string& address, int port, bool ssl = false);
    WebServer(const WebServer&) = delete;
    WebServer(WebServer&&) = default;
    virtual ~WebServer() = default;

    WebServer& operator=(const WebServer&) = delete;
    WebServer& operator=(WebServer&&) = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Restbed server
    std::shared_ptr<restbed::Service>& server() noexcept { return _server; }
    //! Get the Restbed settings
    std::shared_ptr<restbed::Settings>& settings() noexcept { return _settings; }
    //! Get the Restbed SSL settings
    std::shared_ptr<restbed::SSLSettings>& ssl_settings() noexcept { return _ssl_settings; }

    //! Is the server SSL secured?
    bool IsSSL() const noexcept { return _ssl; }
    //! Is the server started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start();
    //! Stop the server
    /*!
        \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
    */
    bool Stop();
    //! Restart the server
    /*!
        \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
    */
    bool Restart();

protected:
    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

private:
    // Asio service
    std::shared_ptr<Service> _service;
    // Restbed server & settings
    std::shared_ptr<restbed::Service> _server;
    std::shared_ptr<restbed::Settings> _settings;
    std::shared_ptr<restbed::SSLSettings> _ssl_settings;
    std::atomic<bool> _started;
    bool _ssl;
};

/*! \example web_http_server.cpp HTTP Web server example */
/*! \example web_https_server.cpp HTTPS Web server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEB_SERVER_H
