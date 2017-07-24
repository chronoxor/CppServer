/*!
    \file pair_client.h
    \brief Nanomsg pair client definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_PAIR_CLIENT_H
#define CPPSERVER_NANOMSG_PAIR_CLIENT_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg pair client
/*!
    Nanomsg pair client is used to perform a duplex communication with
    the Nanomsg pair server.

    Pair protocol is the simplest and least scalable scalability protocol.
    It allows scaling by breaking the application in exactly two pieces.
    For example, if a monolithic application handles both accounting and
    agenda of HR department, it can be split into two applications
    (accounting vs. HR) that are run on two separate servers.
    These applications can then communicate via PAIR sockets.

    The downside of this protocol is that its scaling properties are very limited.
    Splitting the application into two pieces allows to scale the two servers.
    To add the third server to the cluster, the application has to be split once more,
    say by separating HR functionality into hiring module and salary computation module.
    Whenever possible, try to use one of the more scalable protocols instead.

    Thread-safe.
*/
class PairClient : public Client
{
public:
    //! Initialize client with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run the client in a separate thread (default is true)
    */
    explicit PairClient(const std::string& address, bool threading = true)
        : Client(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Pair, address, threading)
    {}
    PairClient(const PairClient&) = delete;
    PairClient(PairClient&&) noexcept = default;
    virtual ~PairClient() = default;

    PairClient& operator=(const PairClient&) = delete;
    PairClient& operator=(PairClient&&) noexcept = default;
};

/*! \example nanomsg_pair_client.cpp Nanomsg pair client example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_PAIR_CLIENT_H
