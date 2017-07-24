/*!
    \file web_server.h
    \brief HTTP Web server definition
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

//! HTTP Web server
/*!
    HTTP Web server is used to provide HTTP interface to handle different
    kind of Web requests such as POST, GET, PUT, DELETE, etc.

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class WebServer : public std::enable_shared_from_this<WebServer>
{
public:
    //! Initialize HTTP Web server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
    */
    explicit WebServer(std::shared_ptr<Service> service, int port);
    //! Initialize HTTP Web server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit WebServer(std::shared_ptr<Service> service, const std::string& address, int port);
    WebServer(const WebServer&) = delete;
    WebServer(WebServer&&) noexcept = default;
    virtual ~WebServer() = default;

    WebServer& operator=(const WebServer&) = delete;
    WebServer& operator=(WebServer&&) noexcept = default;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Restbed server
    std::shared_ptr<restbed::Service>& server() noexcept { return _server; }
    //! Get the Restbed settings
    std::shared_ptr<restbed::Settings>& settings() noexcept { return _settings; }

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
    std::atomic<bool> _started;
};

/*! \example web_server.cpp HTTP Web server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_WEB_SERVER_H
