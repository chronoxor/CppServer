/*!
    \file timer.h
    \brief Timer definition
    \author Ivan Shynkarenka
    \date 16.08.2018
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_TIMER_H
#define CPPSERVER_ASIO_TIMER_H

#include "service.h"

#include "time/time.h"
#include "time/timespan.h"

#include <cassert>
#include <functional>

namespace CppServer {
namespace Asio {

//! Timer
/*!
    Timer is used to plan and perform delayed operation.

    Thread-safe.
*/
class Timer : public std::enable_shared_from_this<Timer>
{
public:
    //! Initialize timer with a given Asio service
    /*!
        \param service - Asio service
    */
    Timer(const std::shared_ptr<Service>& service);
    //! Initialize timer with a given Asio service and absolute expiry time
    /*!
        \param service - Asio service
        \param time - Absolute time
    */
    Timer(const std::shared_ptr<Service>& service, const CppCommon::UtcTime& time);
    //! Initialize timer with a given Asio service and expiry time relative to now
    /*!
        \param service - Asio service
        \param timespan - Relative timespan
    */
    Timer(const std::shared_ptr<Service>& service, const CppCommon::Timespan& timespan);
    //! Initialize timer with a given Asio service and action function
    /*!
        \param service - Asio service
        \param action - Action function
    */
    Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action);
    //! Initialize timer with a given Asio service, action function and absolute expiry time
    /*!
        \param service - Asio service
        \param action - Action function
        \param time - Absolute time
    */
    Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action, const CppCommon::UtcTime& time);
    //! Initialize timer with a given Asio service, action function and expiry time relative to now
    /*!
        \param service - Asio service
        \param action - Action function
        \param timespan - Relative timespan
    */
    Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action, const CppCommon::Timespan& timespan);
    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    virtual ~Timer() = default;

    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    //! Get the Asio service
    std::shared_ptr<Service>& service() noexcept { return _service; }
    //! Get the Asio IO service
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    //! Get the Asio service strand for serialized handler execution
    asio::io_service::strand& strand() noexcept { return _strand; }

    //! Get the timer's expiry time as an absolute time
    CppCommon::UtcTime expire_time() const;
    //! Get the timer's expiry time relative to now
    CppCommon::Timespan expire_timespan() const;

    //! Setup the timer with absolute expiry time
    /*!
        \param time - Absolute time
        \return 'true' if the timer was successfully setup, 'false' if the timer failed to setup
    */
    virtual bool Setup(const CppCommon::UtcTime& time);
    //! Setup the timer with expiry time relative to now
    /*!
        \param timespan - Relative timespan
        \return 'true' if the timer was successfully setup, 'false' if the timer failed to setup
    */
    virtual bool Setup(const CppCommon::Timespan& timespan);
    //! Setup the timer with an action function
    /*!
        \param action - Action function
        \return 'true' if the timer was successfully setup, 'false' if the timer failed to setup
    */
    virtual bool Setup(const std::function<void(bool)>& action);
    //! Setup the timer with an action function and absolute expiry time
    /*!
        \param action - Action function
        \param time - Absolute time
        \return 'true' if the timer was successfully setup, 'false' if the timer failed to setup
    */
    virtual bool Setup(const std::function<void(bool)>& action, const CppCommon::UtcTime& time);
    //! Setup the timer with an action function and expiry time relative to now
    /*!
        \param action - Action function
        \param timespan - Relative timespan
        \return 'true' if the timer was successfully setup, 'false' if the timer failed to setup
    */
    virtual bool Setup(const std::function<void(bool)>& action, const CppCommon::Timespan& timespan);

    //! Wait for the timer (asynchronous)
    /*!
        \return 'true' if the timer was successfully expired, 'false' if any error occurred
    */
    virtual bool WaitAsync();
    //! Wait for the timer (synchronous)
    /*!
        \return 'true' if the timer was successfully expired, 'false' if any error occurred
    */
    virtual bool WaitSync();

    //! Cancel any wait operation on the timer
    /*!
        \return 'true' if the timer was successfully canceled, 'false' if any error occurred
    */
    virtual bool Cancel();

protected:
    //! Handle timer notification
    virtual void onTimer(bool canceled) {}

    //! Handle error notification
    /*!
        \param error - Error code
        \param category - Error category
        \param message - Error message
    */
    virtual void onError(int error, const std::string& category, const std::string& message) {}

private:
    // Asio service
    std::shared_ptr<Service> _service;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialized handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Deadline timer
    asio::system_timer _timer;
    // Action function
    std::function<void(bool)> _action;

    //! Send error notification
    void SendError(std::error_code ec);
    //! Send timer notification
    void SendTimer(bool canceled);
};

/*! \example asio_timer.cpp Asio timer example */

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_ASIO_TIMER_H
