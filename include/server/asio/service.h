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
#include "memory.h"

#include "threads/thread.h"

#include <atomic>
#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace CppServer {
namespace Asio {

//! Asio service
/*!
    Asio service is used to host all clients/servers based on Asio C++ library.
    It is implemented based on Asio C++ Library and use one or more threads to
    perform all asynchronous IO operations and communications.

    There are three ways to initialize Asio service:

    1) Service(threads, false) - initialize a new Asio service with io-context-
    per-thread design. In this case each Asio IO context will be bounded to its
    own working thread and all handlers will be dispatched sequentially without
    strands making the code clean and easy to maintain.

    2) Service(threads, true) - initialize a new Asio service with thread-pool
    design. In this case single Asio IO context will be bounded to all threads
    in pool, but strands will be required to serialize handler execution.

    3) Service(service, true | false) - initialize a new Asio service using the
    existing Asio IO context instance with required strands flag. Strands are
    required for serialized handler execution when single Asio IO context used
    in thread pool.

    Thread-safe.

    https://think-async.com
*/
class Service : public std::enable_shared_from_this<Service>
{
public:
    //! Initialize Asio service with single or multiple working threads
    /*!
        \param threads - Working threads count (default is 1)
        \param pool - Asio service thread pool flag (default is false)
    */
    explicit Service(int threads = 1, bool pool = false);
    //! Initialize Asio service manually with a given Asio IO context
    /*!
        \param io_context - Asio IO context
        \param strands - Asio IO context strands required flag (default is false)
    */
    explicit Service(const std::shared_ptr<asio::io_context>& io_context, bool strands = false);
    Service(const Service&) = delete;
    Service(Service&&) = delete;
    virtual ~Service() = default;

    Service& operator=(const Service&) = delete;
    Service& operator=(Service&&) = delete;

    //! Get the number of working threads
    size_t threads() const noexcept { return _threads.size(); }

    //! Is the service required strand to serialized handler execution?
    bool IsStrandRequired() const noexcept { return _strand_required; }
    //! Is the service started with polling loop mode?
    bool IsPolling() const noexcept { return _polling; }
    //! Is the service started?
    bool IsStarted() const noexcept { return _started; }

    //! Start the service
    /*!
        \param polling - Polling loop mode with idle handler call (default is false)
        \return 'true' if the service was successfully started, 'false' if the service failed to start
    */
    virtual bool Start(bool polling = false);
    //! Stop the service
    /*!
        \return 'true' if the service was successfully stopped, 'false' if the service is already stopped
    */
    virtual bool Stop();
    //! Restart the service
    /*!
        \return 'true' if the service was successfully restarted, 'false' if the service failed to restart
    */
    virtual bool Restart();

    //! Get the next available Asio IO context
    /*!
        Method will return single Asio IO context for manual or thread pool design or
        will return the next available Asio IO context using round-robin algorithm for
        io-service-per-thread design.

        \return Asio IO context
    */
    virtual std::shared_ptr<asio::io_context>& GetAsioContext() noexcept
    { return _io_contexts[++_round_robin_index % _io_contexts.size()]; }

    //! Dispatch the given handler
    /*!
        The given handler may be executed immediately if this function is called from IO context thread.
        Otherwise it will be enqueued to the IO context pending operations queue.

        Method takes a handler to dispatch as a parameter and returns async result of the handler.
    */
    template <typename CompletionHandler>
    ASIO_INITFN_RESULT_TYPE(CompletionHandler, void()) Dispatch(ASIO_MOVE_ARG(CompletionHandler) handler)
    { if (_strand_required) return asio::dispatch(*_strand, handler); else return asio::dispatch(*_io_contexts[0], handler); }

    //! Post the given handler
    /*!
        The given handler will be enqueued to the IO context pending operations queue.

        Method takes a handler to dispatch as a parameter and returns async result of the handler.
    */
    template <typename CompletionHandler>
    ASIO_INITFN_RESULT_TYPE(CompletionHandler, void()) Post(ASIO_MOVE_ARG(CompletionHandler) handler)
    { if (_strand_required) return asio::post(*_strand, handler); else return asio::post(*_io_contexts[0], handler); }

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
    // Asio IO contexts
    std::vector<std::shared_ptr<asio::io_context>> _io_contexts;
    // Asio service working threads
    std::vector<std::thread> _threads;
    // Asio service strand for serialized handler execution
    std::shared_ptr<asio::io_context::strand> _strand;
    // Asio service strands required flag
    std::atomic<bool> _strand_required;
    // Asio service polling loop mode flag
    std::atomic<bool> _polling;
    // Asio service state
    std::atomic<bool> _started;
    std::atomic<size_t> _round_robin_index;

    //! Service thread
    static void ServiceThread(const std::shared_ptr<Service>& service, const std::shared_ptr<asio::io_context>& io_context);

    //! Send error notification
    void SendError(std::error_code ec);
};

/*! \example asio_service.cpp Asio service example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_SERVICE_H
