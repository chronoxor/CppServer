/*!
    \file client.cpp
    \brief Nanomsg client implementation
    \author Ivan Shynkarenka
    \date 28.01.2017
    \copyright MIT License
*/

#include "server/nanomsg/client.h"

#include "errors/fatal.h"
#include "threads/thread.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Client::Client(Domain domain, Protocol protocol, const std::string& address, bool threading)
    : _address(address),
      _socket(domain, protocol),
      _connected(false),
      _threading(threading),
      _joining(false)
{
    // Start the client thread
    if (_threading)
        _thread = CppCommon::Thread::Start([this]() { ClientLoop(); });
}

Client::~Client()
{
    if (IsConnected())
        Disconnect();

    // Wait for client thread
    if (_threading)
    {
        _joining = true;
        if (_thread.joinable())
            _thread.join();
    }
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
            _connected = true;

            // Call the client connected handler
            onConnected();

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

bool Client::Disconnect()
{
    assert(IsConnected() && "Nanomsg client is not connected!");
    if (!IsConnected())
        return false;

    try
    {
        if (_socket.Disconnect())
        {
            _connected = false;

            // Call the client disconnected handler
            onDisconnected();

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

bool Client::Reconnect()
{
    if (!Disconnect())
        return false;

    return Connect();
}

void Client::ClientLoop()
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        while (!_joining)
        {
            // Run Nanomsg client in a loop
            while (IsConnected())
            {
                // Try to receive a new message from the server
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
        fatality("Nanomsg client thread terminated!");
    }

    // Call the cleanup thread handler
    onThreadCleanup();
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
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
        onError(ex.system_error(), ex.string());
        return 0;
    }
}

} // namespace Nanomsg
} // namespace CppServer
