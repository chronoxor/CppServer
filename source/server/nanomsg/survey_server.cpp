/*!
    \file survey_server.cpp
    \brief Nanomsg survey server implementation
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/survey_server.h"

#include "errors/exceptions.h"

namespace CppServer {
namespace Nanomsg {

std::tuple<size_t, bool> SurveyServer::ReceiveSurvey(Message& message)
{
    if (!IsStarted())
        return std::make_tuple(0, true);

    try
    {
        return socket().ReceiveSurvey(message);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
        return std::make_tuple(0, true);
    }
}

std::tuple<size_t, bool> SurveyServer::TryReceiveSurvey(Message& message)
{
    if (!IsStarted())
        return std::make_tuple(0, true);

    try
    {
        return socket().TryReceiveSurvey(message);
    }
    catch (CppCommon::SystemException& ex)
    {
        onError(ex.system_error(), ex.string());
        return std::make_tuple(0, true);
    }
}

} // namespace Nanomsg
} // namespace CppServer
