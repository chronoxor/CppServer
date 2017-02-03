/*!
    \file respondent_client.h
    \brief Nanomsg respondent client definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_RESPONDENT_CLIENT_H
#define CPPSERVER_NANOMSG_RESPONDENT_CLIENT_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg respondent client
/*!
    Nanomsg respondent client is used to response for survey messges from
    the Nanomsg surveyor server.

    Use to respond to the survey. Survey is received using receive function,
    response is sent using send function. This socket can be connected to at
    most one peer.

    Thread-safe.
*/
class RespondentClient : public Client
{
public:
    //! Initialize client with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run client in a separate thread (default is true)
    */
    explicit RespondentClient(const std::string& address, bool threading = true)
        : Client(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Respondent, address, threading)
    {}
    RespondentClient(const RespondentClient&) = delete;
    RespondentClient(RespondentClient&&) = default;
    virtual ~RespondentClient() = default;

    RespondentClient& operator=(const RespondentClient&) = delete;
    RespondentClient& operator=(RespondentClient&&) = default;
};

/*! \example nanomsg_respondent_client.cpp Nanomsg respondent client example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_RESPONDENT_CLIENT_H
