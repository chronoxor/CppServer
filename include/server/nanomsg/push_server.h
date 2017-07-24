/*!
    \file push_server.h
    \brief Nanomsg push server definition
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_PUSH_SERVER_H
#define CPPSERVER_NANOMSG_PUSH_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg push server
/*!
    Nanomsg push server is used to pull messages from the Nanomsg push
    clients.

    This server is used to receive a message from a cluster of nodes.
    Send operation is not implemented on this server type.

    Thread-safe.
*/
class PushServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run the server in a separate thread (default is true)
    */
    explicit PushServer(const std::string& address, bool threading = true)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Pull, address, threading)
    {}
    PushServer(const PushServer&) = delete;
    PushServer(PushServer&&) noexcept = default;
    virtual ~PushServer() = default;

    PushServer& operator=(const PushServer&) = delete;
    PushServer& operator=(PushServer&&) noexcept = default;

private:
    using Server::Send;
    using Server::TrySend;
};

/*! \example nanomsg_push_server.cpp Nanomsg push server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_PUSH_SERVER_H
