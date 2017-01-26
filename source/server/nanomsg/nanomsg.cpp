/*!
    \file nanomsg.cpp
    \brief Nanomsg C++ Library implementation
    \author Ivan Shynkarenka
    \date 26.01.2017
    \copyright MIT License
*/

#include "server/nanomsg/nanomsg.h"

namespace CppServer {
namespace Nanomsg {

std::ostream& operator<<(std::ostream& stream, Domain domain)
{
    switch (domain)
    {
        case Domain::Std:
            return stream << "Std";
        case Domain::Raw:
            return stream << "Raw";
        default:
            return stream << "<unknown>";
    }
}

std::ostream& operator<<(std::ostream& stream, Protocol protocol)
{
    switch (protocol)
    {
        case Protocol::Pair:
            return stream << "Pair";
        case Protocol::Request:
            return stream << "Request";
        case Protocol::Reply:
            return stream << "Reply";
        case Protocol::Publisher:
            return stream << "Publisher";
        case Protocol::Subscriber:
            return stream << "Subscriber";
        case Protocol::Push:
            return stream << "Push";
        case Protocol::Pull:
            return stream << "Pull";
        case Protocol::Surveyor:
            return stream << "Surveyor";
        case Protocol::Respondent:
            return stream << "Respondent";
        case Protocol::Bus:
            return stream << "Bus";
        default:
            return stream << "<unknown>";
    }
}

} // namespace Nanomsg
} // namespace CppServer
