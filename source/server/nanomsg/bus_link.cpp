/*!
    \file bus_link.cpp
    \brief Nanomsg bus link implementation
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_link.h"

namespace CppServer {
namespace Nanomsg {

bool BusLink::Link(const std::string& address)
{
    try
    {
        return socket().Link(address);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.system_message());
        return false;
    }
}

} // namespace Nanomsg
} // namespace CppServer
