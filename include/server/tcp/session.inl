/*!
    \file session.inl
    \brief TCP session inline implementation
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

namespace CppServer {

template <class TServer, class TSession>
inline TCPSession<TServer, TSession>::TCPSession(TServer& server, asio::ip::tcp::socket socket) : _server(server), _socket(std::move(socket))
{
}

} // namespace CppServer
