/*!
    \file server.cpp
    \brief Nanomsg server implementation
    \author Ivan Shynkarenka
    \date 27.01.2017
    \copyright MIT License
*/

#include "server/nanomsg/server.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Server::Server(Domain domain, Protocol protocol, const std::string& address, bool threading)
    : _address(address),
      _socket(domain, protocol),
      _threading(threading)
{
}

Server::~Server()
{
    if (IsStarted())
        Stop();
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

bool Server::Stop()
{
    assert(IsStarted() && "Nanomsg server is not started!");
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

bool Server::Restart()
{
    if (!Stop())
        return false;

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
            Message message;

            // Try to receive a new message from the client
            if (TryReceive(message) == 0)
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
