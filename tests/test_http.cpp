//
// Created by Ivan Shynkarenka on 08.05.2019
//

#include "test.h"

#include "server/http/http_client.h"
#include "server/http/http_server.h"
#include "threads/thread.h"

#include <map>
#include <mutex>

using namespace CppCommon;
using namespace CppServer::Asio;
using namespace CppServer::HTTP;

class Cache : public CppCommon::Singleton<Cache>
{
   friend CppCommon::Singleton<Cache>;

public:
    bool GetCache(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            return true;
        }
        else
            return false;
    }

    void SetCache(std::string_view key, std::string_view value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.emplace(key, value);
        if (!it.second)
            it.first->second = value;
    }

    bool DeleteCache(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            _cache.erase(it);
            return true;
        }
        else
            return false;
    }

private:
    std::mutex _cache_lock;
    std::map<std::string, std::string, std::less<>> _cache;
};

class HTTPCacheSession : public CppServer::HTTP::HTTPSession
{
public:
    using CppServer::HTTP::HTTPSession::HTTPSession;

protected:
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override
    {
        // Process HTTP request methods
        if (request.method() == "HEAD")
            SendResponseAsync(response().MakeHeadResponse());
        else if (request.method() == "GET")
        {
            // Get the cache value
            std::string cache;
            if (Cache::GetInstance().GetCache(request.url(), cache))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(cache));
            }
            else
                SendResponseAsync(response().MakeErrorResponse("Required cache value was not found for the key: " + std::string(request.url())));
        }
        else if ((request.method() == "POST") || (request.method() == "PUT"))
        {
            // Set the cache value
            Cache::GetInstance().SetCache(request.url(), request.body());
            // Response with the cache value
            SendResponseAsync(response().MakeOKResponse());
        }
        else if (request.method() == "DELETE")
        {
            // Delete the cache value
            std::string cache;
            if (Cache::GetInstance().DeleteCache(request.url(), cache))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(cache));
            }
            else
                SendResponseAsync(response().MakeErrorResponse("Deleted cache value was not found for the key: " + std::string(request.url())));
        }
        else if (request.method() == "OPTIONS")
            SendResponseAsync(response().MakeOptionsResponse());
        else if (request.method() == "TRACE")
            SendResponseAsync(response().MakeTraceResponse(request.cache()));
        else
            SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP method: " + std::string(request.method())));
    }

    void onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) override
    {
        std::cout << "Request error: " << error << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class HTTPCacheServer : public CppServer::HTTP::HTTPServer
{
public:
    using CppServer::HTTP::HTTPServer::HTTPServer;

protected:
    std::shared_ptr<TCPSession> CreateSession(std::shared_ptr<TCPServer> server) override
    {
        return std::make_shared<HTTPCacheSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

TEST_CASE("HTTP server & client test", "[CppServer][HTTP]")
{
    // HTTP server address and port
    std::string address = "127.0.0.1";
    int port = 4444;

    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and start HTTP server
    auto server = std::make_shared<HTTPCacheServer>(service, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create a new HTTP client
    auto client = std::make_shared<CppServer::HTTP::HTTPClientEx>(service, address, "http");

    // Test CRUD operations
    auto response = client->SendGetRequest("/test").get();
    REQUIRE(response.status() == 500);
    response = client->SendPostRequest("/test", "old_value").get();
    REQUIRE(response.status() == 200);
    response = client->SendGetRequest("/test").get();
    REQUIRE(response.status() == 200);
    REQUIRE(response.body() == "old_value");
    response = client->SendPutRequest("/test", "new_value").get();
    REQUIRE(response.status() == 200);
    response = client->SendGetRequest("/test").get();
    REQUIRE(response.status() == 200);
    REQUIRE(response.body() == "new_value");
    response = client->SendDeleteRequest("/test").get();
    REQUIRE(response.status() == 200);
    REQUIRE(response.body() == "new_value");
    response = client->SendGetRequest("/test").get();
    REQUIRE(response.status() == 500);

    // Stop the HTTP server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();
}
