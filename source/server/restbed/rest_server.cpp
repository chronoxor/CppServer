/*!
    \file rest_server.cpp
    \brief REST server implementation
    \author Ivan Shynkarenka
    \date 13.03.2017
    \copyright MIT License
*/

#include "server/restbed/rest_server.h"

#include "errors/fatal.h"

namespace CppServer {
namespace Restbed {

RestServer::RestServer(std::shared_ptr<restbed::Service> service, std::shared_ptr<restbed::Settings> settings)
    : _service(service),
      _settings(settings),
      _started(false)
{
    assert((service != nullptr) && "Restbed service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Restbed service is invalid!");

    assert((settings != nullptr) && "Restbed settings are invalid!");
    if (settings == nullptr)
        throw CppCommon::ArgumentException("Restbed settings are invalid!");
}

bool RestServer::Start()
{
    assert(!IsStarted() && "REST server is already started!");
    if (IsStarted())
        return false;

    // Set ready handle
    auto self(this->shared_from_this());
    _service->set_ready_handler([this, self](restbed::Service&)
    {
         // Update the started flag
        _started = true;

        // Call the service started handler
        onStarted();
    });

    // Start the service thread
    _thread = CppCommon::Thread::Start([this]() { ServerLoop(); });

    return true;
}

bool RestServer::Stop()
{
    assert(IsStarted() && "REST server is not started!");
    if (!IsStarted())
        return false;

    // Stop the Restbed service
    _service->stop();

    // Update the started flag
    _started = false;

    // Call the service stopped handler
    onStopped();

    // Wait for server thread
    _thread.join();

    return true;
}

bool RestServer::Restart()
{
    if (!Stop())
        return false;

    return Start();
}

void RestServer::ServerLoop()
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        // Start the Restbed service
        _service->start(_settings);
    }
    catch (...)
    {
        fatality("REST server thread terminated!");
    }

    // Call the cleanup thread handler
    onThreadCleanup();
}

} // namespace Restbed
} // namespace CppServer
