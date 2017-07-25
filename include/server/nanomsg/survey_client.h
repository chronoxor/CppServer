/*!
    \file survey_client.h
    \brief Nanomsg survey client definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SURVEY_CLIENT_H
#define CPPSERVER_NANOMSG_SURVEY_CLIENT_H

#include "client.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg survey client
/*!
    Nanomsg survey client is used to response for survey messges from
    the Nanomsg survey server.

    Use to respond to the survey. Survey is received using receive function,
    response is sent using send function. This socket can be connected to at
    most one peer.

    Thread-safe.
*/
class SurveyClient : public Client
{
public:
    //! Initialize client with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run the client in a separate thread (default is true)
    */
    explicit SurveyClient(const std::string& address, bool threading = true)
        : Client(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Respondent, address, threading)
    {}
    SurveyClient(const SurveyClient&) = delete;
    SurveyClient(SurveyClient&&) = default;
    virtual ~SurveyClient() = default;

    SurveyClient& operator=(const SurveyClient&) = delete;
    SurveyClient& operator=(SurveyClient&&) = default;
};

/*! \example nanomsg_survey_client.cpp Nanomsg survey client example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SURVEY_CLIENT_H
