/*!
    \file subscribe_server.h
    \brief Nanomsg subscribe server definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SUBSCRIBE_SERVER_H
#define CPPSERVER_NANOMSG_SUBSCRIBE_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg subscribe server
/*!
    Nanomsg subscribe server is used to publish messages for all
    Nanomsg subscriber clients.

    This server is used to distribute messages to multiple destinations.
    Receive operation is not defined.

    Thread-safe.
*/
class SubscribeServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
    */
    explicit SubscribeServer(const std::string& address)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Publisher, address, false)
    {}
    SubscribeServer(const SubscribeServer&) = delete;
    SubscribeServer(SubscribeServer&&) = default;
    virtual ~SubscribeServer() = default;

    SubscribeServer& operator=(const SubscribeServer&) = delete;
    SubscribeServer& operator=(SubscribeServer&&) = default;

private:
    using Server::Receive;
    using Server::TryReceive;
    using Server::onThreadInitialize;
    using Server::onThreadCleanup;
    using Server::onIdle;
    using Server::onReceived;
};

/*! \example nanomsg_subscribe_server.cpp Nanomsg subscribe server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SUBSCRIBE_SERVER_H
