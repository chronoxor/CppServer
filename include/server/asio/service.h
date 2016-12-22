/*!
    \file service.h
    \brief Asio service definition
    \author Ivan Shynkarenka
    \date 16.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_SERVICE_H
#define CPPSERVER_ASIO_SERVICE_H

#include "threads/thread.h"

#include "asio.h"

#include <atomic>
#include <memory>
#include <string>

namespace CppServer {
namespace Asio {

//! Asio service
/*!
    Asio service is used to host all clients/servers based on Asio C++ library.
    It is implemented based on Asio C++ Library and use a separate thread to
    perform all asynchronous IO operations and communications.

    Thread-safe.
*/
class Service : public std::enable_shared_from_this<Service>
{
public:
    Service() : _started(false) {}
    Service(const Service&) = delete;
    Service(Service&&) = default;
    virtual ~Service() { Stop(); }

    Service& operator=(const Service&) = delete;
    Service& operator=(Service&&) = default;

    //! Get the Asio service
    asio::io_service& service() noexcept { return _service; }

    //! Is the service started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the service
    /*!
        \param polling - Polling loop mode with idle handler call (default is false)
        \return 'true' if the service was successfully started, 'false' if the service failed to start
    */
    bool Start(bool polling = false);
    //! Stop the service
    /*!
        \return 'true' if the service was successfully stopped, 'false' if the service is already stopped
    */
    bool Stop();

protected:
    //! Initialize thread handler
    /*!
         This handler can be used to initialize priority or affinity of the service thread.
    */
    virtual void onThreadInitialize() {}
    //! Cleanup thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the service thread.
    */
    virtual void onThreadCleanup() {}

    //! Handle service started notification
    virtual void onStarted() {}
    //! Handle service stopped notification
    virtual void onStopped() {}

    //! Handle service idle notification
    virtual void onIdle() { CppCommon::Thread::Yield(); }

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Asio service
    asio::io_service _service;
    std::thread _thread;
    std::atomic<bool> _started;

    //! Service loop
    void ServiceLoop(bool polling);
};

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_SERVICE_H
