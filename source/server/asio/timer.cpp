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

Timer::Timer(std::shared_ptr<Service> service)
    : _service(service),
      _io_service(_service->GetAsioService()),
      _strand(*_io_service),
      _strand_required(_service->IsStrandRequired()),
      _timer(*_io_service)
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

Timer::Timer(std::shared_ptr<Service> service, const CppCommon::UtcTime& time)
    : _service(service),
    _io_service(_service->GetAsioService()),
    _strand(*_io_service),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_service, time.chrono())
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

Timer::Timer(std::shared_ptr<Service> service, const CppCommon::Timespan& timespan)
    : _service(service),
    _io_service(_service->GetAsioService()),
    _strand(*_io_service),
    _strand_required(_service->IsStrandRequired()),
    _timer(*_io_service, timespan.chrono())
{
    assert((service != nullptr) && "Asio service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("Asio service is invalid!");
}

CppCommon::UtcTime Timer::expire_time() const
{
    return CppCommon::UtcTime::chrono(_timer.expires_at());
}

CppCommon::Timespan Timer::expire_timespan() const
{
    return CppCommon::Timespan::chrono(_timer.expires_from_now());
}

bool Timer::Setup(const CppCommon::UtcTime& time)
{
    asio::error_code ec;
    _timer.expires_at(time.chrono(), ec);

    // Check for error
    if (ec)
    {
        SendError(ec);
        return false;
    }

    return true;
}

bool Timer::Setup(const CppCommon::Timespan& timespan)
{
    asio::error_code ec;
    _timer.expires_from_now(timespan.chrono(), ec);

    // Check for error
    if (ec)
    {
        SendError(ec);
        return false;
    }

    return true;
}

bool Timer::WaitAsync()
{
    auto self(this->shared_from_this());
    auto async_wait_handler = [this, self](const std::error_code& ec)
    {
        // Call the timer aborted handler
        if (ec == asio::error::operation_aborted)
            onTimer(true);

        // Check for error
        if (ec)
        {
            SendError(ec);
            return;
        }

        // Call the timer expired handler
        onTimer(false);
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

    // Check for error
    if (ec)
    {
        SendError(ec);
        return false;
    }

    return true;
}

bool Timer::Cancel()
{
    asio::error_code ec;
    _timer.cancel(ec);

    // Check for error
    if (ec)
    {
        SendError(ec);
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

} // namespace Asio
} // namespace CppServer
