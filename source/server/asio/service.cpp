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

    // Post started routine
    _service.post([this]()
    {
        // Update started flag
        _started = true;

        // Call service started handler
        onStarted();
    });

    // Start service thread
    _thread = std::thread([this, polling]() { ServiceLoop(polling); });
}

void Service::Stop()
{
    if (!IsStarted())
        return;

    // Post stopped routine
    _service.post([this]()
    {
        // Update started flag
        _started = false;

        // Call service stopped handler
        onStopped();

        // Stop Asio service
        _service.stop();
    });

    // Wait for service thread
    _thread.join();
}

void Service::ServiceLoop(bool polling)
{
    // Call initialize thread handler
    onThreadInitialize();

    try
    {
        if (polling)
        {
            // Run Asio service in a polling loop
            do
            {
                // Poll all pending handlers
                _service.poll();

                // Call idle handler
                onIdle();
            } while (_started);
        }
        else
        {
            // Run Asio service in a running loop
            do
            {
                // Run all pending handlers
                _service.run();

                // Call idle handler
                onIdle();
            } while (_started);
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

    // Call cleanup thread handler
    onThreadCleanup();
}

} // namespace Asio
} // namespace CppServer
