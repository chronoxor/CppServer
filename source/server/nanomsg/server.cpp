/*!
    \file server.cpp
    \brief Nanomsg server implementation
    \author Ivan Shynkarenka
    \date 27.01.2016
    \copyright MIT License
*/

#include "server/nanomsg/server.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Server::Server(Domain domain, Protocol protocol, const std::string& address)
    : _address(address),
      _socket(domain, protocol)
{
}

bool Server::Start()
{
    assert(!IsStarted() && "Nanomsg server is already started!");
    if (IsStarted())
        return false;

    try
    {
        if (_socket.Bind(_address))
        {
            // Call the server started handler
            onStarted();
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

bool Server::StartThread()
{
    // Start the server
    if (!Start())
        return false;

    // Start the server thread
    _thread = CppCommon::Thread::Start([this]() { ServerLoop(); });

    return true;
}

bool Server::Stop()
{
    assert(IsStarted() && "Nanomsg server is not started!");
    if (!IsStarted())
        return false;

    bool result = true;

    try
    {
        if (_socket.Disconnect())
        {
            // Call the server stopped handler
            onStopped();
        }
        else
            result = false;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        result = false;
    }

    // Wait for server thread
    if (_thread.joinable())
        _thread.join();

    return result;
}

bool Server::Restart()
{
    bool start_thread = _thread.joinable();

    if (!Stop())
        return false;

    if (start_thread)
        return StartThread();
    else
        return Start();
}

void Server::ServerLoop()
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        // Run Nanomsg server in a loop
        do
        {
            // Try to receive a new message from the client
            Message message;
            if (!TryReceive(message))
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
        fatality("Nanomsg server thread terminated!");
    }

    // Call the cleanup thread handler
    onThreadCleanup();
}

size_t Server::Send(const void* buffer, size_t size)
{
    if (!IsStarted())
        return 0;

    try
    {
        return _socket.Send(buffer, size);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

size_t Server::TrySend(const void* buffer, size_t size)
{
    if (!IsStarted())
        return 0;

    try
    {
        return _socket.TrySend(buffer, size);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

size_t Server::Receive(Message& message)
{
    if (!IsStarted())
        return 0;

    try
    {
        size_t result = _socket.Receive(message);
        if (result > 0)
        {
            // Call the message received handler
            onReceived(message);
        }
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

size_t Server::TryReceive(Message& message)
{
    if (!IsStarted())
        return 0;

    try
    {
        size_t result = _socket.TryReceive(message);
        if (result > 0)
        {
            // Call the message received handler
            onReceived(message);
        }
        return result;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return 0;
    }
}

} // namespace Nanomsg
} // namespace CppServer
