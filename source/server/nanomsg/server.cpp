/*!
    \file server.cpp
    \brief Nanomsg server implementation
    \author Ivan Shynkarenka
    \date 27.01.2016
    \copyright MIT License
*/

#include "server/nanomsg/server.h"

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

bool Server::Stop()
{
    assert(IsStarted() && "Nanomsg server is not started!");
    if (!IsStarted())
        return false;

    try
    {
        if (_socket.Disconnect())
        {
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

    while (IsStarted())
        CppCommon::Thread::Yield();

    return Start();
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
