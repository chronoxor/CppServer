/*!
    \file rest_server.h
    \brief REST server definition
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_RESTBED_REST_SERVER_H
#define CPPSERVER_RESTBED_REST_SERVER_H

#include "threads/thread.h"

#include <restbed>

#include <atomic>
#include <cassert>
#include <memory>

namespace CppServer {
namespace Restbed {

//! REST server
/*!
    REST server is used to provide RESTfull interface to handle http requests
    (POST, GET, PUT, DELETE, etc).

    Thread-safe.

    https://github.com/corvusoft/restbed
*/
class RestServer : public std::enable_shared_from_this<RestServer>
{
public:
    //! Initialize REST server with a given Restbed service and settings
    /*!
        \param service - Restbed service
        \param settings - Restbed settings
    */
    explicit RestServer(std::shared_ptr<restbed::Service> service, std::shared_ptr<restbed::Settings> settings);
    RestServer(const RestServer&) = delete;
    RestServer(RestServer&&) = default;
    virtual ~RestServer() = default;

    RestServer& operator=(const RestServer&) = delete;
    RestServer& operator=(RestServer&&) = default;

    //! Get the Restbed service
    std::shared_ptr<restbed::Service>& service() noexcept { return _service; }
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
    //! Initialize thread handler
    /*!
         This handler can be used to initialize priority or affinity of the server thread.
    */
    virtual void onThreadInitialize() {}
    //! Cleanup thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the server thread.
    */
    virtual void onThreadCleanup() {}

    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle service idle notification
    virtual void onIdle() { CppCommon::Thread::Yield(); }

private:
    // Restbed service & settings
    std::shared_ptr<restbed::Service> _service;
    std::shared_ptr<restbed::Settings> _settings;
    std::thread _thread;
    std::atomic<bool> _started;

    //! Server loop
    void ServerLoop();
};

/*! \example rest_http_server.cpp REST http server example */
/*! \example rest_https_server.cpp REST https server example */

} // namespace Restbed
} // namespace CppServer

#endif // CPPSERVER_RESTBED_REST_SERVER_H
