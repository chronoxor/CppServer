/*!
    \file service.h
    \brief Asio service definition
    \author Ivan Shynkarenka
    \date 16.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_SERVICE_H
#define CPPSERVER_ASIO_SERVICE_H

#include "asio.h"

#include "threads/thread.h"

#include <atomic>
#include <cassert>
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

    http://think-async.com
*/
class Service : public std::enable_shared_from_this<Service>
{
public:
    //! Initialize a new Asio service
    Service();
    //! Initialize Asio service with a given Asio service
    /*!
        \param service - Asio service
    */
    explicit Service(std::shared_ptr<asio::io_service> service);
    Service(const Service&) = delete;
    Service(Service&&) noexcept = default;
    virtual ~Service() = default;

    Service& operator=(const Service&) = delete;
    Service& operator=(Service&&) noexcept = default;

    //! Get the Asio service
    std::shared_ptr<asio::io_service>& service() noexcept { return _service; }

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
    //! Restart the service
    /*!
        \return 'true' if the service was successfully restarted, 'false' if the service failed to restart
    */
    bool Restart();

    //! Dispatch the given handler
    /*!
        The given handler may be executed immediately if this function is called from IO service thread.
        Otherwise it will be enqueued to the IO service pending operations queue.

        \param handler - Handler to dispatch
        \return Async result of the handler
    */
    template <typename CompletionHandler>
    ASIO_INITFN_RESULT_TYPE(CompletionHandler, void()) Dispatch(ASIO_MOVE_ARG(CompletionHandler) handler)
    { return _service->dispatch(handler); }

    //! Post the given handler
    /*!
        The given handler will be enqueued to the IO service pending operations queue.

        \param handler - Handler to dispatch
        \return Async result of the handler
    */
    template <typename CompletionHandler>
    ASIO_INITFN_RESULT_TYPE(CompletionHandler, void()) Post(ASIO_MOVE_ARG(CompletionHandler) handler)
    { return _service->post(handler); }

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
    std::shared_ptr<asio::io_service> _service;
    std::thread _thread;
    std::atomic<bool> _started;

    //! Service loop
    void ServiceLoop(bool polling);

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example asio_service.cpp Asio service example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_SERVICE_H
