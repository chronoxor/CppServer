/*!
    \file session.h
    \brief TCP session definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_TCP_SESSION_H
#define CPPSERVER_TCP_SESSION_H

#include "../../modules/asio/asio/include/asio.hpp"

#include <utility>

namespace CppServer {

//! TCP session
/*!
    TCP session is used to read and write data from the connected TCP client.

    Not thread-safe.
*/
template <class TServer>
class TCPSession
{
public:
    //! Initialize TCP session with a given TCP server and connected socket
    /*!
        \param server - TCP server
        \param socket - Connected socket
    */
    explicit TCPSession(TServer& server, asio::tcp::socket socket);
    TCPSession(const TCPSession&) = delete;
    TCPSession(TCPSession&&) noexcept = default;
    virtual ~TCPSession() {};

    TCPSession& operator=(const TCPSession&) = delete;
    TCPSession& operator=(TCPSession&&) noexcept = default;

protected:
    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    TServer& _server;
    asio::tcp::socket _socket;
};

} // namespace CppServer

#include "session.inl"

#endif // CPPSERVER_TCP_SESSION_H
