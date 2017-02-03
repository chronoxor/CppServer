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
      _threading(threading)
{
}

Client::~Client()
{
    if (IsConnected())
        Disconnect();
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

            // Start the client thread
            if (_threading)
                _thread = CppCommon::Thread::Start([this]() { ClientLoop(); });

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
            // Wait for client thread
            if (_threading)
                if (_thread.joinable())
                    _thread.join();

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

    return Connect();
}

void Client::ClientLoop()
{
    // Call the initialize thread handler
    onThreadInitialize();

    try
    {
        // Run Nanomsg client in a loop
        do
        {
            Message message;

            // Try to receive a new message from the server
            if (!TryReceive(message))
            {
                // Call the idle handler
                onIdle();
            }
        } while (IsConnected());
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
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
