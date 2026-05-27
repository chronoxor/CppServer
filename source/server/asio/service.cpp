/*!
    \file service.cpp
    \brief Asio service implementation
    \author Ivan Shynkarenka
    \date 16.12.2016
    \copyright MIT License
*/

#include "server/asio/service.h"

#include "errors/fatal.h"

namespace CppServer {
namespace Asio {

Service::Service(int threads, bool pool)
    : _strand_required(false),
      _polling(false),
      _started(false),
      _round_robin_index(0)
{
    assert((threads >= 0) && "Working threads counter must not be negative!");

    if (threads == 0)
    {
        // Single Asio IO context without thread pool
        _io_contexts.emplace_back(std::make_shared<asio::io_context>());
    }
    else if (!pool)
    {
        // IO-context-per-thread design
        for (int thread = 0; thread < threads; ++thread)
        {
            _io_contexts.emplace_back(std::make_shared<asio::io_context>());
            _threads.emplace_back(std::thread());
        }
    }
    else
    {
        // Thread-pool design
        _io_contexts.emplace_back(std::make_shared<asio::io_context>());
        for (int thread = 0; thread < threads; ++thread)
            _threads.emplace_back(std::thread());
        _strand = std::make_shared<asio::io_context::strand>(*_io_contexts[0]);
        _strand_required = true;
    }
}

Service::Service(const std::shared_ptr<asio::io_context>& io_context, bool strands)
    : _strand_required(strands),
      _polling(false),
      _started(false),
      _round_robin_index(0)
{
    assert((io_context != nullptr) && "Asio IO context is invalid!");
    if (io_context == nullptr)
        throw CppCommon::ArgumentException("Asio IO context is invalid!");

    _io_contexts.emplace_back(io_context);
    if (_strand_required)
        _strand = std::make_shared<asio::io_context::strand>(*_io_contexts[0]);
}

bool Service::Start(bool polling)
{
    assert(!IsStarted() && "Asio service is already started!");
    if (IsStarted())
        return false;

    // Update polling loop mode flag
    _polling = polling;

    // Reset round robin index
    _round_robin_index = 0;

    // Post the started handler
    auto self(this->shared_from_this());
    auto start_handler = [this, self]()
    {
        if (IsStarted())
            return;

         // Update the started flag
        _started = true;

        // Call the service started handler
        onStarted();
    };
    if (_strand_required)
        asio::post(*_strand, start_handler);
    else
        asio::post(*_io_contexts[0], start_handler);

    // Start service working threads
    for (size_t thread = 0; thread < _threads.size(); ++thread)
        _threads[thread] = CppCommon::Thread::Start([this, self, thread]() { ServiceThread(self, _io_contexts[thread % _io_contexts.size()]); });

    // Wait for service is started
    while (!IsStarted())
        CppCommon::Thread::Yield();

    return true;
}

bool Service::Stop()
{
    assert(IsStarted() && "Asio service is not started!");
    if (!IsStarted())
        return false;

    // Post the stop routine
    auto self(this->shared_from_this());
    auto stop_handler = [this, self]()
    {
        if (!IsStarted())
            return;

        // Stop Asio IO contexts
        for (auto& io_context : _io_contexts)
            io_context->stop();

        // Update the started flag
        _started = false;

        // Call the service stopped handler
        onStopped();
    };
    if (_strand_required)
        asio::post(*_strand, stop_handler);
    else
        asio::post(*_io_contexts[0], stop_handler);

    // Wait for all service working threads
    for (auto& thread : _threads)
        thread.join();

    // Update polling loop mode flag
    _polling = false;

    // Wait for service is stopped
    while (IsStarted())
        CppCommon::Thread::Yield();

    return true;
}

bool Service::Restart()
{
    bool polling = IsPolling();

    if (!Stop())
        return false;

    // Reinitialize new Asio IO contexts
    for (size_t i = 0; i < _io_contexts.size(); ++i)
        _io_contexts[i] = std::make_shared<asio::io_context>();
    if (_strand_required)
        _strand = std::make_shared<asio::io_context::strand>(*_io_contexts[0]);

    return Start(polling);
}

void Service::ServiceThread(const std::shared_ptr<Service>& service, const std::shared_ptr<asio::io_context>& io_context)
{
    bool polling = service->IsPolling();

    // Call the initialize thread handler
    service->onThreadInitialize();

    try
    {
        // Attach the current working thread to the Asio context
        auto work = asio::make_work_guard(*io_context);

        // Service loop...
        do
        {
            // ...with handling some specific Asio errors
            try
            {
                if (polling)
                {
                    // Poll all pending handlers
                    io_context->poll();

                    // Call the idle handler
                    service->onIdle();
                }
                else
                {
                    // Run all pending handlers
                    io_context->run();
                    break;
                }
            }
            catch (const asio::system_error& ex)
            {
                std::error_code ec = ex.code();

                // Skip Asio disconnect errors
                if (ec == asio::error::not_connected)
                    continue;

                throw;
            }
        } while (service->IsStarted());
    }
    catch (const asio::system_error& ex)
    {
        service->SendError(ex.code());
    }
    catch (const std::exception& ex)
    {
        fatality(ex);
    }
    catch (...)
    {
        fatality("Asio service thread terminated!");
    }

    // Call the cleanup thread handler
    service->onThreadCleanup();

#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    // Delete OpenSSL thread state
    OPENSSL_thread_stop();
#endif
}

void Service::SendError(std::error_code ec)
{
    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
