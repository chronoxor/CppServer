/*!
    \file pull_server.h
    \brief Nanomsg pull server definition
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_PULL_SERVER_H
#define CPPSERVER_NANOMSG_PULL_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg pull server
/*!
    Nanomsg pull server is used to pull messages from the Nanomsg clients.

    This server is used to receive a message from a cluster of nodes.
    Send operation is not implemented on this server type.

    Thread-safe.
*/
class PullServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run server in a separate thread (default is true)
    */
    explicit PullServer(const std::string& address, bool threading = true)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Pull, address, threading)
    {
    }
    PullServer(const PullServer&) = delete;
    PullServer(PullServer&&) = default;
    virtual ~PullServer() = default;

    PullServer& operator=(const PullServer&) = delete;
    PullServer& operator=(PullServer&&) = default;

private:
    using Server::Send;
    using Server::TrySend;
};

/*! \example nanomsg_pull_server.cpp Nanomsg pull server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_PULL_SERVER_H
