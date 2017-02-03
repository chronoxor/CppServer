/*!
    \file subscribe_client.h
    \brief Nanomsg subscribe client definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SUBSCRIBE_CLIENT_H
#define CPPSERVER_NANOMSG_SUBSCRIBE_CLIENT_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg subscribe client
/*!
    Nanomsg subscribe client is used to receive published messages from
    the Nanomsg subscribe server.

    Receives messages from the publisher. Only messages that the socket is
    subscribed to are received. When the socket is created there are no
    subscriptions and thus no messages will be received. Send operation
    is not defined on this socket.

    Thread-safe.
*/
class SubscribeClient : public Client
{
public:
    //! Initialize client with a given endpoint address
    /*!
        \param address - Endpoint address
        \param topic - Subscription topic (default is "")
        \param threading - Run the client in a separate thread (default is true)
    */
    explicit SubscribeClient(const std::string& address, const std::string& topic = "", bool threading = true)
        : Client(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Subscriber, address, threading)
    {
        socket().SetSocketOption(NN_SUB, NN_SUB_SUBSCRIBE, topic.data(), topic.size());
    }
    SubscribeClient(const SubscribeClient&) = delete;
    SubscribeClient(SubscribeClient&&) = default;
    virtual ~SubscribeClient() = default;

    SubscribeClient& operator=(const SubscribeClient&) = delete;
    SubscribeClient& operator=(SubscribeClient&&) = default;

private:
    using Client::Send;
    using Client::TrySend;
};

/*! \example nanomsg_subscribe_client.cpp Nanomsg subscribe client example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SUBSCRIBE_CLIENT_H
