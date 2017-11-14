/*!
    \file bus.cpp
    \brief Nanomsg bus node implementation
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus.h"

namespace CppServer {
namespace Nanomsg {

bool Bus::Link(const std::string& address)
{
    if (!IsStarted())
        return false;

    try
    {
        return socket().Link(address);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
        return false;
    }
}

} // namespace Nanomsg
} // namespace CppServer
