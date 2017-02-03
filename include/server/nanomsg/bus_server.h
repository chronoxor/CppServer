/*!
    \file bus_server.h
    \brief Nanomsg bus server definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_BUS_SERVER_H
#define CPPSERVER_NANOMSG_BUS_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg pair server
/*!
    Nanomsg bus server is used to perform a duplex communication with
    other the Nanomsg bus servers.

    Broadcasts messages from any node to all other nodes in the topology.
    The server should never receive messages that it sent itself.

    This pattern scales only to local level (within a single machine
    or within a single LAN). Trying to scale it further can result in
    overloading individual nodes with messages.

    Sent messages are distributed to all nodes in the topology. Incoming
    messages from all other nodes in the topology are fair-queued in the
    server.

    Warning: For bus topology to function correctly, user is responsible
    for ensuring that path from each node to any other node exists within
    the topology.

    Thread-safe.
*/
class BusServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run server in a separate thread (default is true)
    */
    explicit BusServer(const std::string& address, bool threading = true)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Bus, address, threading)
    {}
    BusServer(const BusServer&) = delete;
    BusServer(BusServer&&) = default;
    virtual ~BusServer() = default;

    BusServer& operator=(const BusServer&) = delete;
    BusServer& operator=(BusServer&&) = default;

    //! Connect the bus server to the remote endpoint
    /*!
        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the bus server was successfully connected, 'false' if the bus server was already connected or the nanomsg engine terminated
    */
    bool Connect(const std::string& address);
};

/*! \example nanomsg_bus_server.cpp Nanomsg bus server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_BUS_SERVER_H
