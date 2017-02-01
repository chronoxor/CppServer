/*!
    \file push_client.h
    \brief Nanomsg push client definition
    \author Ivan Shynkarenka
    \date 01.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_PUSH_CLIENT_H
#define CPPSERVER_NANOMSG_PUSH_CLIENT_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg push client
/*!
    Nanomsg push client is used to push one way messages to Nanomsg server.

    Thread-safe.
*/
class PushClient : public Client
{
public:
    //! Initialize client with a given endpoint address
    /*!
        \param address - Endpoint address
    */
    explicit PushClient(const std::string& address)
        : Client(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Push, address, false)
    {}
    PushClient(const PushClient&) = delete;
    PushClient(PushClient&&) = default;
    virtual ~PushClient() = default;

    PushClient& operator=(const PushClient&) = delete;
    PushClient& operator=(PushClient&&) = default;

private:
    using Client::Receive;
    using Client::TryReceive;
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_PUSH_CLIENT_H
