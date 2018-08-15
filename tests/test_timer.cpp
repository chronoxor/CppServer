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
    std::atomic<bool> canceled;
    std::atomic<bool> expired;
    std::atomic<bool> errors;

    AsioTimer(std::shared_ptr<Service> service)
        : Timer(service),
          canceled(false),
          expired(false),
          errors(false)
    {
    }

protected:
    void onTimer(bool aborted) override
    {
        if (aborted)
            canceled = true;
        else
            expired = true;
    }

    void onError(int error, const std::string& category, const std::string& message) override { errors = true; }
};

} // namespace

TEST_CASE("Asio timer", "[CppServer][Asio]")
{
    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create Asio timer
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

    // Check the timer state
    REQUIRE(timer->canceled);
    REQUIRE(timer->expired);
    REQUIRE(!timer->errors);
}
