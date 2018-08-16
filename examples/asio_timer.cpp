/*!
    \file asio_timer.cpp
    \brief Asio timer example
    \author Ivan Shynkarenka
    \date 16.08.2018
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/asio/timer.h"
#include "threads/thread.h"

#include <iostream>

class AsioTimer : public CppServer::Asio::Timer
{
public:
    using CppServer::Asio::Timer::Timer;

protected:
    void onTimer(bool canceled) override
    {
        std::cout << "Asio timer " << (canceled ? "canceled" : "expired") << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Asio timer caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new Asio timer
    auto timer = std::make_shared<AsioTimer>(service);

    // Setup and synchronously wait for the timer
    timer->Setup(CppCommon::UtcTime() + CppCommon::Timespan::seconds(1));
    timer->WaitSync();

    // Setup and asynchronously wait for the timer
    timer->Setup(CppCommon::Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    CppCommon::Thread::Sleep(2000);

    // Setup and asynchronously wait for the timer
    timer->Setup(CppCommon::Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    CppCommon::Thread::Sleep(500);

    // Cancel the timer
    timer->Cancel();

    // Wait for a while...
    CppCommon::Thread::Sleep(500);

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
