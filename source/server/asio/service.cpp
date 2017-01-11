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

bool Service::Start(bool polling)
{
    if (IsStarted())
        return false;

    // Post the started routine
    auto self(this->shared_from_this());
    _service.post([this, self]()
    {
         // Update the started flag
        _started = true;

        // Call the service started handler
        onStarted();
    });

    // Start the service thread
    _thread = std::thread([this, polling]() { ServiceLoop(polling); });

    return true;
}

bool Service::Stop()
{
    if (!IsStarted())
        return false;

    // Post the stop routine
    auto self(this->shared_from_this());
    _service.post([this, self]()
    {
        // Stop the Asio service
        _service.stop();

        // Update the started flag
        _started = false;

        // Call the service stopped handler
        onStopped();
    });

    // Wait for service thread
    _thread.join();

    return true;
}

bool Service::Restart()
{
    if (!Stop())
        return false;

    return Start();
}

void Service::ServiceLoop(bool polling)
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        asio::io_service::work work(_service);

        if (polling)
        {
            // Run the Asio service in a polling loop
            do
            {
                // Poll all pending handlers
                _service.poll();

                // Call the idle handler
                onIdle();
            } while (_started);
        }
        else
        {
            // Run all pending handlers
            _service.run();
        }
    }
    catch (asio::system_error& ex)
    {
        onError(ex.code().value(), ex.code().category().name(), ex.code().message());
    }
    catch (...)
    {
        fatality("TCP service thread terminated!");
    }

    // Call the cleanup thread handler
    onThreadCleanup();
}

} // namespace Asio
} // namespace CppServer
