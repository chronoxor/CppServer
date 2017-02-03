/*!
    \file surveyor_server.h
    \brief Nanomsg surveyor server implementation
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/surveyor_server.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

SurveyorServer::SurveyorServer(const std::string& address, bool threading)
    : _address(address),
      _socket(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Surveyor),
      _threading(threading)
{
}

SurveyorServer::~SurveyorServer()
{
    if (IsStarted())
        Stop();
}

bool SurveyorServer::Start()
{
    assert(!IsStarted() && "Nanomsg surveyor server is already started!");
    if (IsStarted())
        return false;

    try
    {
        if (_socket.Bind(_address))
        {
            // Call the server started handler
            onStarted();

            // Start the server thread
            if (_threading)
                _thread = CppCommon::Thread::Start([this]() { ServerLoop(); });

            return true;
        }
        else
            return false;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return false;
    }
}

bool SurveyorServer::Stop()
{
    assert(IsStarted() && "Nanomsg surveyor server is not started!");
    if (!IsStarted())
        return false;

    try
    {
        if (_socket.Disconnect())
        {
            // Wait for server thread
            if (_threading)
                if (_thread.joinable())
                    _thread.join();

            // Call the server stopped handler
            onStopped();

            return true;
        }
        else
            return false;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return false;
    }
}

bool SurveyorServer::Restart()
{
    if (!Stop())
        return false;

    return Start();
}

void SurveyorServer::ServerLoop()
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        // Run Nanomsg server in a loop
        do
        {
            Message message;

            // Try to receive a new survey respond from clients
            std::tuple<size_t, bool> result = TryReceiveSurvey(message);
            if (std::get<0>(result) == 0)
            {
                // Call the idle handler
                onIdle();
            }
        } while (IsStarted());
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
    }
    catch (...)
    {
        fatality("Nanomsg surveyor server thread terminated!");
    }

    // Call the cleanup thread handler
    onThreadCleanup();
}

size_t SurveyorServer::Survey(const void* buffer, size_t size)
{
    if (!IsStarted())
        return 0;

    try
    {
        size_t result = _socket.Send(buffer, size);
        if (result == size)
            onSurveyStarted(buffer, size);
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

size_t SurveyorServer::TrySurvey(const void* buffer, size_t size)
{
    if (!IsStarted())
        return 0;

    try
    {
        size_t result = _socket.TrySend(buffer, size);
        if (result == size)
            onSurveyStarted(buffer, size);
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

std::tuple<size_t, bool> SurveyorServer::ReceiveSurvey(Message& message)
{
    if (!IsStarted())
        return std::make_tuple(0, true);

    try
    {
        std::tuple<size_t, bool> result = _socket.ReceiveSurvey(message);

        // Call the survey stopped handler
        if (std::get<1>(result))
            onSurveyStopped();
        else if (std::get<0>(result) > 0)
        {
            // Call the message received handler
            onReceived(message);
        }
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return std::make_tuple(0, true);
    }
}

std::tuple<size_t, bool> SurveyorServer::TryReceiveSurvey(Message& message)
{
    if (!IsStarted())
        return std::make_tuple(0, true);

    try
    {
        std::tuple<size_t, bool> result = _socket.TryReceiveSurvey(message);

        // Call the survey stopped handler
        if (std::get<1>(result))
            onSurveyStopped();
        else if (std::get<0>(result) > 0)
        {
            // Call the message received handler
            onReceived(message);
        }
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return std::make_tuple(0, true);
    }
}

} // namespace Nanomsg
} // namespace CppServer
