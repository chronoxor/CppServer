/*!
    \file survey_server.h
    \brief Nanomsg survey server definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SURVEY_SERVER_H
#define CPPSERVER_NANOMSG_SURVEY_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg survey server
/*!
    Nanomsg survey server is used to perform a survey using several
    connected Nanomsg survey clients.

    Used to send the survey. The survey is delivered to all the connected
    respondents. Once the query is sent, the socket can be used to receive
    the responses. When the survey deadline expires, receive will return
    ETIMEDOUT error.

    Thread-safe.
*/
class SurveyServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param deadline - Deadline timeout in milliseconds (default is 1000)
    */
    explicit SurveyServer(const std::string& address, int deadline = 1000)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Surveyor, address, false)
    {
        socket().SetSocketOption(NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &deadline, sizeof(deadline));
    }
    SurveyServer(const SurveyServer&) = delete;
    SurveyServer(SurveyServer&&) noexcept = default;
    virtual ~SurveyServer() = default;

    SurveyServer& operator=(const SurveyServer&) = delete;
    SurveyServer& operator=(SurveyServer&&) noexcept = default;

    //! Receive a respond to the survey from the clients in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> ReceiveSurvey(Message& message);

    //! Try to receive a respond to the survey from the clients in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> TryReceiveSurvey(Message& message);

private:
    using Server::Receive;
    using Server::TryReceive;
    using Server::onThreadInitialize;
    using Server::onThreadCleanup;
    using Server::onIdle;
    using Server::onReceived;
};

/*! \example nanomsg_survey_server.cpp Nanomsg survey server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SURVEY_SERVER_H
