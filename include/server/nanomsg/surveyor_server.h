/*!
    \file surveyor_server.h
    \brief Nanomsg surveyor server definition
    \author Ivan Shynkarenka
    \date 02.02.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_SURVEYOR_SERVER_H
#define CPPSERVER_NANOMSG_SURVEYOR_SERVER_H

#include "socket.h"

#include "threads/thread.h"

#include <thread>

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
class SurveyorServer
{
public:
    //! Initialize server with a given endpoint address
    /*!
        \param address - Endpoint address
        \param threading - Run server in a separate thread (default is true)
    */
    explicit SurveyorServer(const std::string& address, bool threading = true);
    SurveyorServer(const SurveyorServer&) = delete;
    SurveyorServer(SurveyorServer&&) = default;
    virtual ~SurveyorServer();

    SurveyorServer& operator=(const SurveyorServer&) = delete;
    SurveyorServer& operator=(SurveyorServer&&) = default;

    //! Get the Nanomsg socket
    Socket& socket() noexcept { return _socket; }

    //! Is the server started?
    bool IsStarted() const noexcept { return _socket.IsOpened() && _socket.IsConnected(); }

    //! Start the server
    /*!
        \return 'true' if the server was successfully started, 'false' if the server failed to start
    */
    bool Start();
    //! Stop the server
    /*!
        \return 'true' if the server was successfully stopped, 'false' if the server is already stopped
    */
    bool Stop();
    //! Restart the server
    /*!
        \return 'true' if the server was successfully restarted, 'false' if the server failed to restart
    */
    bool Restart();

    //! Survey data to all respondents
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t Survey(const void* buffer, size_t size);
    //! Survey a text string to all respondents
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t Survey(const std::string& text) { return Survey(text.data(), text.size()); }
    //! Survey a message to all respondents
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t Survey(const Message& message) { return Survey(message.buffer(), message.size()); }

    //! Try to survey data to all respondents in non-blocking mode
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
        \return Count of sent bytes
    */
    size_t TrySurvey(const void* buffer, size_t size);
    //! Try to survey a text string to all respondents in non-blocking mode
    /*!
        \param text - Text string to send
        \return Count of sent bytes
    */
    size_t TrySurvey(const std::string& text) { return TrySurvey(text.data(), text.size()); }
    //! Try to survey a message to all respondents in non-blocking mode
    /*!
        \param message - Message to send
        \return Count of sent bytes
    */
    size_t TrySurvey(const Message& message) { return TrySurvey(message.buffer(), message.size()); }

    //! Receive a respond to the survey from the client in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> ReceiveSurvey(Message& message);

    //! Try to receive a respond to the survey from the client in non-blocking mode
    /*!
        \param message - Message to receive
        \return Count of received bytes and survey complete flag
    */
    std::tuple<size_t, bool> TryReceiveSurvey(Message& message);

protected:
    //! Initialize thread handler
    /*!
         This handler can be used to initialize priority or affinity of the server thread.
    */
    virtual void onThreadInitialize() {}
    //! Cleanup thread handler
    /*!
         This handler can be used to cleanup priority or affinity of the server thread.
    */
    virtual void onThreadCleanup() {}

    //! Handle server started notification
    virtual void onStarted() {}
    //! Handle server stopped notification
    virtual void onStopped() {}

    //! Handle server idle notification
    virtual void onIdle() { CppCommon::Thread::Yield(); }

    //! Handle survey started notification
    /*!
        \param buffer - Buffer to send
        \param size - Buffer size
    */
    virtual void onSurveyStarted(const void* buffer, size_t size) {}
    //! Handle survey stopped notification
    virtual void onSurveyStopped() {}

    //! Handle message received notification
    /*!
        \param message - Received message
    */
    virtual void onReceived(Message& message) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param message - Error message
    */
    virtual void onError(int error, const std::string& message) {}

private:
    // Nanomsg endpoint address
    std::string _address;
    // Nanomsg socket
    Socket _socket;
    // Nanomsg server thread
    bool _threading;
    std::thread _thread;

    //! Server loop
    void ServerLoop();
};

} // namespace Nanomsg
} // namespace CppServer

#endif // CPPSERVER_NANOMSG_SURVEYOR_SERVER_H
