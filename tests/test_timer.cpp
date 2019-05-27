//
// Created by Ivan Shynkarenka on 16.08.2018
//

#include "test.h"

#include "server/asio/timer.h"
#include "threads/thread.h"

using namespace CppCommon;
using namespace CppServer::Asio;

namespace {

class AsioTimer : public Timer
{
public:
    using Timer::Timer;

protected:
    void onTimer(bool aborted) override
    {
        if (aborted)
            canceled = true;
        else
            expired = true;
    }

    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }

public:
    std::atomic<bool> canceled{false};
    std::atomic<bool> expired{false};
    std::atomic<bool> errors{false};
};

} // namespace

TEST_CASE("Asio timer test", "[CppServer][Timer]")
{
    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create Asio timer
    auto timer = std::make_shared<AsioTimer>(service);

    // Setup and synchronously wait for the timer
    timer->Setup(UtcTime() + Timespan::seconds(1));
    timer->WaitSync();

    // Setup and asynchronously wait for the timer
    timer->Setup(Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    Thread::Sleep(2000);

    // Setup and asynchronously wait for the timer
    timer->Setup(Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    Thread::Sleep(500);

    // Cancel the timer
    timer->Cancel();

    // Wait for a while...
    Thread::Sleep(500);

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();

    // Check the timer state
    REQUIRE(timer->canceled);
    REQUIRE(timer->expired);
    REQUIRE(!timer->errors);
}
