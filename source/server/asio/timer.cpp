/*!
    \file timer.cpp
    \brief Timer implementation
    \author Ivan Shynkarenka
    \date 16.08.2018
    \copyright MIT License
*/

#include "server/asio/timer.h"

namespace CppServer {
namespace Asio {

Timer::Timer(const std::shared_ptr<Service>& service)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

Timer::Timer(const std::shared_ptr<Service>& service, const CppCommon::UtcTime& time)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context, time.chrono())
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

Timer::Timer(const std::shared_ptr<Service>& service, const CppCommon::Timespan& timespan)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context, timespan.chrono())
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

Timer::Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context),
    _action(action)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
    assert((action) && "Action function is invalid!");
    if (!action)
        throw CppCommon::ArgumentException("Action function is invalid!");
}

Timer::Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action, const CppCommon::UtcTime& time)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context, time.chrono()),
    _action(action)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
    assert((action) && "Action function is invalid!");
    if (!action)
        throw CppCommon::ArgumentException("Action function is invalid!");
}

Timer::Timer(const std::shared_ptr<Service>& service, const std::function<void(bool)>& action, const CppCommon::Timespan& timespan)
    : _service(service),
    _io_context(_service->GetAsioContext()),
    _strand(*_io_context),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_context, timespan.chrono()),
    _action(action)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
    assert((action) && "Action function is invalid!");
    if (!action)
        throw CppCommon::ArgumentException("Action function is invalid!");
}

CppCommon::UtcTime Timer::expire_time() const
{
    return CppCommon::UtcTime(_timer.expiry());
}

CppCommon::Timespan Timer::expire_timespan() const
{
    return CppCommon::Timespan(_timer.expiry() - asio::system_timer::clock_type::now());
}

bool Timer::Setup(const CppCommon::UtcTime& time)
{
    try
    {
        _timer.expires_at(time.chrono());
    }
    catch (const asio::system_error& ex)
    {
        SendError(ex.code());
        return false;
    }

    return true;
}

bool Timer::Setup(const CppCommon::Timespan& timespan)
{
    try
    {
        _timer.expires_after(timespan.chrono());
    }
    catch (const asio::system_error& ex)
    {
        SendError(ex.code());
        return false;
    }

    return true;
}

bool Timer::Setup(const std::function<void(bool)>& action)
{
    assert((action) && "Action function is invalid!");
    if (!action)
        return false;

    _action = action;
    return true;
}

bool Timer::Setup(const std::function<void(bool)>& action, const CppCommon::UtcTime& time)
{
    assert((action) && "Action function is invalid!");
    if (!action)
        return false;

    _action = action;
    return Setup(time);
}

bool Timer::Setup(const std::function<void(bool)>& action, const CppCommon::Timespan& timespan)
{
    assert((action) && "Action function is invalid!");
    if (!action)
        return false;

    _action = action;
    return Setup(timespan);
}

bool Timer::WaitAsync()
{
    auto self(this->shared_from_this());
    auto async_wait_handler = [this, self](const std::error_code& ec)
    {
        // Call the timer aborted handler
        if (ec == asio::error::operation_aborted)
            SendTimer(true);

        // Check for error
        if (ec)
        {
            SendError(ec);
            return;
        }

        // Call the timer expired handler
        SendTimer(false);
    };
    if (_strand_required)
        _timer.async_wait(bind_executor(_strand, async_wait_handler));
    else
        _timer.async_wait(async_wait_handler);

    return true;
}

bool Timer::WaitSync()
{
    asio::error_code ec;
    _timer.wait(ec);

    // Call the timer aborted handler
    if (ec == asio::error::operation_aborted)
        SendTimer(true);

    // Check for error
    if (ec)
    {
        SendError(ec);
        return false;
    }

    // Call the timer expired handler
    SendTimer(false);

    return true;
}

bool Timer::Cancel()
{
    try
    {
        _timer.cancel();
    }
    catch (const asio::system_error& ex)
    {
        SendError(ex.code());
        return false;
    }

    return true;
}

void Timer::SendError(std::error_code ec)
{
    // Skip Asio abort error
    if (ec == asio::error::operation_aborted)
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

void Timer::SendTimer(bool canceled)
{
    // Call the timer handler
    onTimer(canceled);

    // Call the timer action
    if (_action)
        _action(canceled);
}

} // namespace Asio
} // namespace CppServer
