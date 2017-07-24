/*!
    \file bus.h
    \brief Nanomsg bus node definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_BUS_H
#define CPPSERVER_NANOMSG_BUS_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg bus node
/*!
    Nanomsg bus node is used to perform a duplex communication with
    other Nanomsg bus nodes.

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
class Bus : public Server
{
public:
    //! Initialize bus node with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run the bus node in a separate thread (default is true)
    */
    explicit Bus(const std::string& address, bool threading = true)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Bus, address, threading)
    {}
    Bus(const Bus&) = delete;
    Bus(Bus&&) noexcept = default;
    virtual ~Bus() = default;

    Bus& operator=(const Bus&) = delete;
    Bus& operator=(Bus&&) noexcept = default;

    //! Link the current bus node to another one
    /*!
        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the bus node was successfully linked, 'false' if the bus node was already linked or the nanomsg engine terminated
    */
    bool Link(const std::string& address);
};

/*! \example nanomsg_bus.cpp Nanomsg bus example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_BUS_H
