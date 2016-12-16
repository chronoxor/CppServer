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

void Service::Start(bool polling)
{
    if (IsStarted())
        return;

    // Call service starting handler
    onStarting();

    // Update started flag
    _started = true;

    // Start service thread
    _thread = std::thread([this, polling]() { ServiceLoop(polling); });
}

void Service::Stop()
{
    if (!IsStarted())
        return;

    // Call service stopping handler
    onStopping();

    // Update started flag
    _started = false;

    // Stop Asio service
    _service.stop();

    // Wait for service thread
    _thread.join();
}

void Service::ServiceLoop(bool polling)
{
    // Call initialize thread handler
    onThreadInitialize();

    try
    {
        // Call service started handler
        onStarted();

        if (polling)
        {
            // Run Asio service in a polling loop
            while (_started)
            {
                // Poll all pending handlers
                _service.poll();

                // Call idle handler
                onIdle();
            }
        }
        else
        {
            // Run Asio service in a running loop
            while (_started)
            {
                // Run all pending handlers
                _service.run();

                // Call idle handler
                onIdle();
            }
        }

        // Call service stopped handler
        onStopped();
    }
    catch (asio::system_error& ex)
    {
        onError(ex.code().value(), ex.code().category().name(), ex.code().message());
    }
    catch (...)
    {
        fatality("TCP service thread terminated!");
    }

    // Call cleanup thread handler
    onThreadCleanup();
}

} // namespace Asio
} // namespace CppServer
