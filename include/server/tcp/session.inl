/*!
    \file session.inl
    \brief TCP session inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {

template <class TServer>
TCPSession::TCPSession(TServer& server, asio::tcp::socket socket) : _server(server), _socket(std::move(socket))
{
}

} // namespace CppServer
