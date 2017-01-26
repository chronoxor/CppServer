/*!
    \file socket.h
    \brief Nanomsg socket definition
    \author Ivan Shynkarenka
    \date 26.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SOCKET_H
#define CPPSERVER_NANOMSG_SOCKET_H

#include "nanomsg.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg socket
/*!
    Nanomsg socket is used as a base communication primitive that wraps all
    necessary Nanomsg library API.

    Thread-safe.

    http://nanomsg.org
*/
class Socket
{
public:
    //! Initialize and open socket with a given domain and protocol
    /*!
        \param domain - Domain
        \param protocol - Protocol
    */
    explicit Socket(Domain domain, Protocol protocol);
    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    virtual ~Socket();

    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = default;

    //! Get the socket domain
    Domain domain() noexcept { return _domain; }
    //! Get the socket protocol
    Protocol protocol() noexcept { return _protocol; }
    //! Get the socket handler
    int socket() noexcept { return _socket; }

    //! Is socket opened?
    bool IsOpened() const noexcept { return (_socket >= 0); }

    //! Close the socket
    void Close();

private:
    Domain _domain;
    Protocol _protocol;
    int _socket;
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SOCKET_H
