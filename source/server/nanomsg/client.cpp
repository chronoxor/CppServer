/*!
    \file client.cpp
    \brief Nanomsg client implementation
    \author Ivan Shynkarenka
    \date 28.01.2016
    \copyright MIT License
*/

#include "server/nanomsg/client.h"

#include "threads/thread.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Client::Client(Domain domain, Protocol protocol, const std::string& address)
    : _address(address),
      _socket(domain, protocol)
{
}

bool Client::Connect()
{
    assert(!IsConnected() && "Nanomsg client is already connected!");
    if (IsConnected())
        return false;

    try
    {
        if (_socket.Connect(_address))
        {
            // Call the client connected handler
            onConnected();
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

bool Client::Disconnect()
{
    assert(IsConnected() && "Nanomsg client is not connected!");
    if (!IsConnected())
        return false;

    try
    {
        if (_socket.Disconnect())
        {
            // Call the client disconnected handler
            onDisconnected();
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

bool Client::Reconnect()
{
    if (!Disconnect())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return Connect();
}

size_t Client::Send(const void* buffer, size_t size)
{
    if (!IsConnected())
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

size_t Client::TrySend(const void* buffer, size_t size)
{
    if (!IsConnected())
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

size_t Client::Receive(Message& message)
{
    if (!IsConnected())
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

size_t Client::TryReceive(Message& message)
{
    if (!IsConnected())
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
