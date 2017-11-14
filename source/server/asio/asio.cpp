/*!
    \file asio.cpp
    \brief Asio C++ Library implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/asio/asio.h"

namespace CppServer {
namespace Asio {

std::ostream& operator<<(std::ostream& stream, InternetProtocol protocol)
{
    switch (protocol)
    {
        case InternetProtocol::IPv4:
            return stream << "IPv4";
        case InternetProtocol::IPv6:
            return stream << "IPv6";
        default:
            return stream << "<unknown>";
    }
}

} // namespace Asio
} // namespace CppServer
