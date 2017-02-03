/*!
    \file bus_server.cpp
    \brief Nanomsg bus server implementation
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_server.h"

namespace CppServer {
namespace Nanomsg {

bool BusServer::Connect(const std::string& address)
{
    if (!IsStarted())
        return false;

    try
    {
        return socket().Connect(address);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return false;
    }
}

} // namespace Nanomsg
} // namespace CppServer
