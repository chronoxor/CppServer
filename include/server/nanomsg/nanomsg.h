/*!
    \file nanomsg.h
    \brief Nanomsg C++ Library definition
    \author Ivan Shynkarenka
    \date 26.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_H
#define CPPSERVER_NANOMSG_H

#define NN_STATIC_LIB

#include <nn.h>

// Nanomsg protocols
#include <bus.h>
#include <pair.h>
#include <pipeline.h>
#include <pubsub.h>
#include <reqrep.h>
#include <survey.h>

// Nanomsg transport
#include <inproc.h>
#include <ipc.h>
#include <tcp.h>
#include <ws.h>

#include <iostream>

namespace CppServer {

/*!
    \namespace CppServer::Nanomsg
    \brief Nanomsg definitions
*/
namespace Nanomsg {

//! Nanomsg domain
enum class Domain
{
    Std = AF_SP,                //!< Standard full-blown socket
    Raw = AF_SP_RAW             //!< Raw socket
};

//! Stream output: Nanomsg domain
/*!
    \param stream - Output stream
    \param protocol - Nanomsg domain
    \return Output stream
*/
std::ostream& operator<<(std::ostream& stream, Domain domain);

//! Nanomsg protocol
enum class Protocol
{
    Pair = NN_PAIR,             //!< Socket for communication with exactly one peer
    Request = NN_REQ,           //!< Used to implement the client application that sends requests and receives replies
    Reply = NN_REP,             //!< Used to implement the stateless worker that receives requests and sends replies
    Publisher = NN_PUB,         //!< Distribute messages to multiple destinations. Receive operation is not defined.
    Subscriber = NN_SUB,        //!< Receives messages from the publisher. Send operation is not defined.
    Push = NN_PUSH,             //!< This socket is used to send messages to a cluster of load-balanced nodes. Receive operation is not implemented on this socket type.
    Pull = NN_PULL,             //!< This socket is used to receive a message from a cluster of nodes. Send operation is not implemented on this socket type.
    Surveyor = NN_SURVEYOR,     //!< Used to send the survey. The survey is delivered to all the connected respondents.
    Respondent = NN_RESPONDENT, //!< Use to respond to the survey. This socket can be connected to at most one peer.
    Bus = NN_BUS                //!< Sent messages are distributed to all nodes in the topology. Incoming messages from all other nodes in the topology are fair-queued in the socket.
};

//! Stream output: Nanomsg protocol
/*!
    \param stream - Output stream
    \param protocol - Nanomsg protocol
    \return Output stream
*/
std::ostream& operator<<(std::ostream& stream, Protocol protocol);

} // namespace Nanomsg

} // namespace CppServer

#endif // CPPSERVER_NANOMSG_H
