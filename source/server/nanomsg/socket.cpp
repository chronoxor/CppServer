/*!
    \file socket.cpp
    \brief Nanomsg socket implementation
    \author Ivan Shynkarenka
    \date 26.01.2017
    \copyright MIT License
*/

#include "server/nanomsg/socket.h"

#include "errors/exceptions.h"
#include "errors/fatal.h"
#include "string/format.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Socket::Socket(Domain domain, Protocol protocol)
    : _domain(domain),
      _protocol(protocol),
      _socket(-1),
      _endpoint(-1)
{
    _socket = nn_socket((int)domain, (int)protocol);
    if (!IsOpened())
        throwex CppCommon::SystemException("Failed to create a new nanomsg socket (domain={}, protocol={})! Nanomsg error: {}"_format(domain, protocol, nn_strerror(nn_errno())));
}

Socket::~Socket()
{
    try
    {
        if (IsConnected())
            Disconnect();
        if (IsOpened())
            Close();
    }
    catch (CppCommon::SystemException& ex)
    {
        fatality(CppCommon::SystemException(ex.string()));
    }
}

uint64_t Socket::established_connections() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_ESTABLISHED_CONNECTIONS);
}

uint64_t Socket::accepted_connections() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_ACCEPTED_CONNECTIONS);
}

uint64_t Socket::dropped_connections() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_DROPPED_CONNECTIONS);
}

uint64_t Socket::broken_connections() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_BROKEN_CONNECTIONS);
}

uint64_t Socket::connect_errors() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_CONNECT_ERRORS);
}

uint64_t Socket::bind_errors() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_BIND_ERRORS);
}

uint64_t Socket::accept_errors() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_ACCEPT_ERRORS);
}

uint64_t Socket::current_connections() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_CURRENT_CONNECTIONS);
}

uint64_t Socket::messages_sent() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_MESSAGES_SENT);
}

uint64_t Socket::messages_received() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_MESSAGES_RECEIVED);
}

uint64_t Socket::bytes_sent() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_BYTES_SENT);
}

uint64_t Socket::bytes_received() const noexcept
{
    return nn_get_statistic(_socket, NN_STAT_BYTES_RECEIVED);
}

bool Socket::SetSocketOption(int level, int option, const void* value, size_t size)
{
    if (!IsOpened())
        return false;

    int result = nn_setsockopt(_socket, level, option, value, size);
    if (result != 0)
    {
        if (nn_errno() == ETERM)
            return false;
        else
            throwex CppCommon::SystemException("Cannot set the nanomsg socket option! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }
    return true;
}

bool Socket::GetSocketOption(int level, int option, void* value, size_t* size)
{
    if (!IsOpened())
        return false;

    int result = nn_getsockopt(_socket, level, option, value, size);
    if (result != 0)
    {
        if (nn_errno() == ETERM)
            return false;
        else
            throwex CppCommon::SystemException("Cannot get the nanomsg socket option! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }
    return true;
}

bool Socket::Bind(const std::string& address)
{
    if (!IsOpened())
        return false;

    if (IsConnected())
        return false;

    int result = nn_bind(_socket, address.c_str());
    if (result < 0)
    {
        if (nn_errno() == ETERM)
            return false;
        else
            throwex CppCommon::SystemException("Cannot bind the nanomsg socket to the given endpoint '{}'! Nanomsg error: {}"_format(address, nn_strerror(nn_errno())));
    }
    _endpoint = result;
    _address = address;
    return true;
}

bool Socket::Connect(const std::string& address)
{
    if (!IsOpened())
        return false;

    if (IsConnected())
        return false;

    int result = nn_connect(_socket, address.c_str());
    if (result < 0)
    {
        if (nn_errno() == ETERM)
            return false;
        else
            throwex CppCommon::SystemException("Cannot connect the nanomsg socket to the given endpoint '{}'! Nanomsg error: {}"_format(address, nn_strerror(nn_errno())));
    }
    _endpoint = result;
    _address = address;
    return true;
}

bool Socket::Disconnect()
{
    if (!IsOpened())
        return false;

    if (!IsConnected())
        return false;

    int result = nn_shutdown(_socket, _endpoint);
    if (result != 0)
    {
        if (nn_errno() == ETERM)
            return false;
        else
            throwex CppCommon::SystemException("Cannot disconnect the nanomsg socket from the endpoint '{}'! Nanomsg error: {}"_format(_address, nn_strerror(nn_errno())));
    }
    _endpoint = -1;
    _address = "";
    return true;
}

size_t Socket::Send(const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsOpened())
        return 0;

    if (!IsConnected())
        return 0;

    int result = nn_send(_socket, buffer, size, 0);
    if (result < 0)
    {
        if (nn_errno() == ETERM)
            return 0;
        else
            throwex CppCommon::SystemException("Cannot send {} bytes to the nanomsg socket! Nanomsg error: {}"_format(size, nn_strerror(nn_errno())));
    }
    return size;
}

size_t Socket::TrySend(const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsOpened())
        return 0;

    if (!IsConnected())
        return 0;

    int result = nn_send(_socket, buffer, size, NN_DONTWAIT);
    if (result < 0)
    {
        if (nn_errno() == EAGAIN)
            return 0;
        else if (nn_errno() == ETERM)
            return 0;
        else
            throwex CppCommon::SystemException("Cannot send {} bytes to the nanomsg socket in non-blocking mode! Nanomsg error: {}"_format(size, nn_strerror(nn_errno())));
    }
    return size;
}

size_t Socket::Receive(Message& message)
{
    if (!IsOpened())
        return 0;

    if (!IsConnected())
        return 0;

    void* data = nullptr;
    int result = nn_recv(_socket, &data, NN_MSG, 0);
    if (result < 0)
    {
        if (nn_errno() == ETERM)
            return 0;
        else
            throwex CppCommon::SystemException("Cannot receive a message from the nanomsg socket! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }

    message._buffer = (uint8_t*)data;
    message._size = result;
    return message.size();
}

size_t Socket::TryReceive(Message& message)
{
    if (!IsOpened())
        return 0;

    if (!IsConnected())
        return 0;

    void* data = nullptr;
    int result = nn_recv(_socket, &data, NN_MSG, NN_DONTWAIT);
    if (result < 0)
    {
        if (nn_errno() == EAGAIN)
            return 0;
        else if (nn_errno() == ETERM)
            return 0;
        else
            throwex CppCommon::SystemException("Cannot receive a message from the nanomsg socket in non-blocking mode! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }

    message._buffer = (uint8_t*)data;
    message._size = result;
    return message.size();
}

std::tuple<size_t, bool> Socket::ReceiveSurvey(Message& message)
{
    if (!IsOpened())
        return std::make_tuple(0, true);

    if (!IsConnected())
        return std::make_tuple(0, true);

    void* data = nullptr;
    int result = nn_recv(_socket, &data, NN_MSG, 0);
    if (result < 0)
    {
        if (nn_errno() == ETIMEDOUT)
            return std::make_tuple(0, true);
        else if (nn_errno() == ETERM)
            return std::make_tuple(0, true);
        else
            throwex CppCommon::SystemException("Cannot receive a survey respond from the nanomsg socket! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }

    message._buffer = (uint8_t*)data;
    message._size = result;
    return std::make_tuple(message.size(), false);
}

std::tuple<size_t, bool> Socket::TryReceiveSurvey(Message& message)
{
    if (!IsOpened())
        return std::make_tuple(0, true);

    if (!IsConnected())
        return std::make_tuple(0, true);

    void* data = nullptr;
    int result = nn_recv(_socket, &data, NN_MSG, NN_DONTWAIT);
    if (result < 0)
    {
        if (nn_errno() == EAGAIN)
            return std::make_tuple(0, false);
        else if (nn_errno() == ETIMEDOUT)
            return std::make_tuple(0, true);
        else if (nn_errno() == ETERM)
            return std::make_tuple(0, true);
        else
            throwex CppCommon::SystemException("Cannot receive a survey respond from the nanomsg socket in non-blocking mode! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    }

    message._buffer = (uint8_t*)data;
    message._size = result;
    return std::make_tuple(message.size(), false);
}

bool Socket::Close()
{
    if (!IsOpened())
        return false;

    int result = nn_close(_socket);
    if (result != 0)
        throwex CppCommon::SystemException("Cannot close the nanomsg socket! Nanomsg error: {}"_format(nn_strerror(nn_errno())));
    _socket = -1;
    _endpoint = -1;
    _address = "";
    return true;
}

void Socket::Terminate()
{
    nn_term();
}

} // namespace Nanomsg
} // namespace CppServer
