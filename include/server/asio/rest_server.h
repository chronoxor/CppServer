/*!
    \file rest_server.h
    \brief REST server definition
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_REST_SERVER_H
#define CPPSERVER_ASIO_REST_SERVER_H

#include "asio.h"
#include "rest.h"
#include "service.h"

namespace CppServer {
namespace Asio {

//! REST server
/*!
    REST server is used to provide HTTP/HTTPS interface to handle different kind
    of web requests such as POST, GET, PUT, DELETE, etc.

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class RestServer : public std::enable_shared_from_this<RestServer>
{
public:
    //! Initialize REST server with a given Asio service and port number
    /*!
        \param service - Asio service
        \param port - Port number
    */
    explicit RestServer(std::shared_ptr<Service> service, int port);
    //! Initialize REST server with a given Asio service, IP address and port number
    /*!
        \param service - Asio service
        \param address - IP address
        \param port - Port number
    */
    explicit RestServer(std::shared_ptr<Service> service, const std::string& address, int port);
    RestServer(const RestServer&) = delete;
    RestServer(RestServer&&) = default;
    virtual ~RestServer() = default;

    RestServer& operator=(const RestServer&) = delete;
    RestServer& operator=(RestServer&&) = default;

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

/*! \example rest_http_server.cpp REST HTTP server example */
/*! \example rest_https_server.cpp REST HTTPS server example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_REST_SERVER_H
