/*!
    \file asio.inl
    \brief Asio C++ Library inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, InternetProtocol protocol)
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
