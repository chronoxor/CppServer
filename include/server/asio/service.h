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

namespace CppServer {
namespace Asio {

//! Asio service
/*!
    Asio service is used to host all clients/servers based on Asio C++ library.

    Not thread-safe.
*/
class Service
{
public:
    Service() : _started(false) {}
    Service(const Service&) = delete;
    Service(Service&&) = default;
    virtual ~Service() = default;

    Service& operator=(const Service&) = delete;
    Service& operator=(Service&&) = default;

    //! Get the Asio service
    asio::io_service& service() noexcept { return _service; }

    //! Is the service started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the service
    /*!
        \param polling - Polling loop mode with idle handler call (default is false)
    */
    void Start(bool polling = false);
    //! Stop the service
    void Stop();

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

    //! Handle service starting notification
    virtual void onStarting() {}
    //! Handle service started notification
    virtual void onStarted() {}

    //! Handle service stopping notification
    virtual void onStopping() {}
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
