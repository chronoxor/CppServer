/*!
    \file publisher_server.h
    \brief Nanomsg publisher server definition
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_PUBLISHER_SERVER_H
#define CPPSERVER_NANOMSG_PUBLISHER_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg publisher server
/*!
    Nanomsg publisher server is used to publish messages for all
    Nanomsg subscribers.

    This server is used to distribute messages to multiple destinations.
    Receive operation is not defined.

    Thread-safe.
*/
class PublisherServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
    */
    explicit PublisherServer(const std::string& address)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Publisher, address, false)
    {}
    PublisherServer(const PublisherServer&) = delete;
    PublisherServer(PublisherServer&&) = default;
    virtual ~PublisherServer() = default;

    PublisherServer& operator=(const PublisherServer&) = delete;
    PublisherServer& operator=(PublisherServer&&) = default;

private:
    using Server::Receive;
    using Server::TryReceive;
    using Server::onThreadInitialize;
    using Server::onThreadCleanup;
    using Server::onIdle;
    using Server::onReceived;
};

/*! \example nanomsg_publisher_server.cpp Nanomsg publisher server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_PUBLISHER_SERVER_H
