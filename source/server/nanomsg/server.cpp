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
      _connected(false),
      _threading(threading),
      _joining(false)
{
    // Start the server thread
    if (_threading)
        _thread = CppCommon::Thread::Start([this]() { ServerLoop(); });
}

Server::~Server()
{
    if (IsStarted())
        Stop();

    // Wait for server thread
    if (_threading)
    {
        _joining = true;
        if (_thread.joinable())
            _thread.join();
    }
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
            _connected = true;

            // Call the server started handler
            onStarted();

            return true;
        }
        else
            return false;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
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
            _connected = false;

            // Call the server stopped handler
            onStopped();

            return true;
        }
        else
            return false;
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
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
        while (!_joining)
        {
            // Run Nanomsg server in a loop
            while (IsStarted())
            {
                // Try to receive a new message from the client
                Message message;
                if (TryReceive(message) == 0)
                {
                    // Call the idle handler
                    onIdle();
                }
            }

            // Switch to another thread...
            CppCommon::Thread::Yield();
        }
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
    }
    catch (std::exception& ex)
    {
        fatality(ex);
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
        return 0;
    }
}

} // namespace Nanomsg
} // namespace CppServer
