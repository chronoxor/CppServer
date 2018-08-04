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
            stream << "IPv4";
        case InternetProtocol::IPv6:
            stream << "IPv6";
        default:
            stream << "<unknown>";
    }
    return stream;
}

} // namespace Asio
} // namespace CppServer
