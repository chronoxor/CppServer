/*!
    \file surveyor_server.h
    \brief Nanomsg surveyor server definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SURVEYOR_SERVER_H
#define CPPSERVER_NANOMSG_SURVEYOR_SERVER_H

#include "server.h"

namespace CppServer {
namespace Nanomsg {

//! Nanomsg surveyor server
/*!
    Nanomsg surveyor server is used to perform a survey using several
    connected Nanomsg respondent clients.

    Used to send the survey. The survey is delivered to all the connected
    respondents. Once the query is sent, the socket can be used to receive
    the responses. When the survey deadline expires, receive will return
    ETIMEDOUT error.

    Thread-safe.
*/
class SurveyorServer : public Server
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param deadline - Deadline timeout in milliseconds (default is 1000)
    */
    explicit SurveyorServer(const std::string& address, int deadline = 1000)
        : Server(CppServer::Nanomsg::Domain::Std, CppServer::Nanomsg::Protocol::Surveyor, address, false)
    {
        socket().SetSocketOption(NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &deadline, sizeof(deadline));
    }
    SurveyorServer(const SurveyorServer&) = delete;
    SurveyorServer(SurveyorServer&&) = default;
    virtual ~SurveyorServer() = default;

    SurveyorServer& operator=(const SurveyorServer&) = delete;
    SurveyorServer& operator=(SurveyorServer&&) = default;

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

/*! \example nanomsg_surveyor_server.cpp Nanomsg surveyor server example */

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SURVEYOR_SERVER_H
