//
// Created by Ivan Shynkarenka on 08.05.2019
//

#include "test.h"

#include "server/http/https_client.h"
#include "server/http/https_server.h"
#include "string/string_utils.h"
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
    std::string GetAllCache()
    {
        std::scoped_lock locker(_cache_lock);
        std::string result;
        result += "[\n";
        for (const auto& item : _cache)
        {
            result += "  {\n";
            result += "    \"key\": \"" + item.first + "\",\n";
            result += "    \"value\": \"" + item.second + "\",\n";
            result += "  },\n";
        }
        result += "]\n";
        return result;
    }

    bool GetCacheValue(std::string_view key, std::string& value)
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

    void PutCacheValue(std::string_view key, std::string_view value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.emplace(key, value);
        if (!it.second)
            it.first->second = value;
    }

    bool DeleteCacheValue(std::string_view key, std::string& value)
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

class HTTPSCacheSession : public HTTPSSession
{
public:
    using HTTPSSession::HTTPSSession;

protected:
    void onReceivedRequest(const HTTPRequest& request) override
    {
        // Process HTTP request methods
        if (request.method() == "HEAD")
            SendResponseAsync(response().MakeHeadResponse());
        else if (request.method() == "GET")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            if (key.empty())
            {
                // Response with all cache values
                SendResponseAsync(response().MakeGetResponse(Cache::GetInstance().GetAllCache(), "application/json; charset=UTF-8"));
            }
            // Get the cache value by the given key
            else if (Cache::GetInstance().GetCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Required cache value was not found for the key: " + key));
        }
        else if ((request.method() == "POST") || (request.method() == "PUT"))
        {
            std::string key(request.url());
            std::string value(request.body());

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Put the cache value
            Cache::GetInstance().PutCacheValue(key, value);

            // Response with the cache value
            SendResponseAsync(response().MakeOKResponse());
        }
        else if (request.method() == "DELETE")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Delete the cache value
            if (Cache::GetInstance().DeleteCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Deleted cache value was not found for the key: " + key));
        }
        else if (request.method() == "OPTIONS")
            SendResponseAsync(response().MakeOptionsResponse());
        else if (request.method() == "TRACE")
            SendResponseAsync(response().MakeTraceResponse(request.cache()));
        else
            SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP method: " + std::string(request.method())));
    }

    void onReceivedRequestError(const HTTPRequest& request, const std::string& error) override
    {
        std::cout << "Request error: " << error << std::endl;
        FAIL();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTPS session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        FAIL();
    }
};

class HTTPSCacheServer : public HTTPSServer
{
public:
    using HTTPSServer::HTTPSServer;

    static std::shared_ptr<SSLContext> CreateContext()
    {
        auto context = std::make_shared<SSLContext>(asio::ssl::context::tlsv13);
        context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
        context->use_certificate_chain_file("../tools/certificates/server.pem");
        context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
        context->use_tmp_dh_file("../tools/certificates/dh4096.pem");
        return context;
    }

protected:
    std::shared_ptr<SSLSession> CreateSession(const std::shared_ptr<SSLServer>& server) override
    {
        return std::make_shared<HTTPSCacheSession>(std::dynamic_pointer_cast<HTTPSServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTPS server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        FAIL();
    }
};

TEST_CASE("HTTPS server & client test", "[CppServer][HTTP]")
{
    // HTTPS server address and port
    std::string address = "127.0.0.1";
    int port = 8443;

    // Create and start Asio service
    auto service = std::make_shared<Service>();
    REQUIRE(service->Start());
    while (!service->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL server context
    auto server_context = HTTPSCacheServer::CreateContext();

    // Create and start HTTPS server
    auto server = std::make_shared<HTTPSCacheServer>(service, server_context, port);
    REQUIRE(server->Start());
    while (!server->IsStarted())
        Thread::Yield();

    // Create and prepare a new SSL client context
    auto client_context = HTTPSCacheServer::CreateContext();

    // Create a new HTTPS client
    auto client = std::make_shared<HTTPSClientEx>(service, client_context, address, port);

    // Test CRUD operations
    auto response = client->SendGetRequest("/test").get();
    REQUIRE(response.status() == 404);
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
    REQUIRE(response.status() == 404);

    // Stop the HTTPS server
    REQUIRE(server->Stop());
    while (server->IsStarted())
        Thread::Yield();

    // Stop the Asio service
    REQUIRE(service->Stop());
    while (service->IsStarted())
        Thread::Yield();
}
