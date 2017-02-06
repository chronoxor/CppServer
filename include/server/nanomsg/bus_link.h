/*!
    \file bus_link.h
    \brief Nanomsg bus link definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_BUS_LINK_H
#define CPPSERVER_NANOMSG_BUS_LINK_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg bus link
/*!
    Nanomsg bus link is used to create a communication link between
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
class BusLink
{
public:
    //! Create a bus link
    BusLink()
        : _socket(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Bus)
    {}
    //! Initialize a bus link with two given endpoint addresses of bus nodes
    /*!
        \param address1 - The first endpoint address of a bus node
        \param address2 - The second endpoint address of a bus node
    */
    explicit BusLink(const std::string& address1, const std::string& address2)
        : BusLink()
    {
        Link(address1);
        Link(address2);
    }
    BusLink(const BusLink&) = delete;
    BusLink(BusLink&&) = default;
    virtual ~BusLink() = default;

    BusLink& operator=(const BusLink&) = delete;
    BusLink& operator=(BusLink&&) = default;

    //! Get the Nanomsg socket
    Socket& socket() noexcept { return _socket; }

    //! Link another bus node
    /*!
        The address argument consists of two parts as follows: transport://address.
        The transport specifies the underlying transport protocol to use.
        The meaning of the address part is specific to the underlying transport protocol.

        \param address - Endpoint address
        \return 'true' if the bus node was successfully linked, 'false' if the bus node was already linked or the nanomsg engine terminated
    */
    bool Link(const std::string& address);

protected:
    //! Handle error notification
    /*!
        \param error - Error code
        \param message - Error message
    */
    virtual void onError(int error, const std::string& message) {}

private:
    // Nanomsg socket
    Socket _socket;
};

/*! \example nanomsg_bus_link.cpp Nanomsg bus link example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_BUS_LINK_H
