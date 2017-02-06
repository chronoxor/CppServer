/*!
    \file bus_node.cpp
    \brief Nanomsg bus node implementation
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_node.h"

namespace CppServer {
namespace Nanomsg {

bool BusNode::Link(const std::string& address)
{
    if (!IsStarted())
        return false;

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
