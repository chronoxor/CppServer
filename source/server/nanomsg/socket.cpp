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
      _socket(-1)
{
    _socket = nn_socket((int)domain, (int)protocol);
    if (!IsOpened())
        throwex CppCommon::SystemException("Failed to create a new nanomsg socket (domain={0}, protocol={1})!"_format(domain, protocol));
}

Socket::~Socket()
{
    if (IsOpened())
    {
        try
        {
            Close();
        }
        catch (CppCommon::SystemException& ex)
        {
            fatality(CppCommon::SystemException(ex.string()));
        }
    }
}

void Socket::Close()
{
    assert(IsOpened() && "Nanomsg socket is not opened!");
    if (!IsOpened())
        throwex CppCommon::SystemException("Nanomsg socket is not opened!");
    int result = nn_close(_socket);
    if (result != 0)
        throwex CppCommon::SystemException("Cannot close the nanomsg socket!");
    _socket = -1;
}

} // namespace Nanomsg
} // namespace CppServer
