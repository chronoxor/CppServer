/*!
    \file http_server.cpp
    \brief HTTP server example
    \author Ivan Shynkarenka
    \date 30.04.2019
    \copyright MIT License
*/

#include "asio_service.h"

#include "server/http/http_server.h"
#include "utility/singleton.h"

#include <iostream>
#include <map>
#include <mutex>

class Cache : public CppCommon::Singleton<Cache>
{
   friend CppCommon::Singleton<Cache>;

public:
    std::string GetCache(std::string_view key)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        return (it != _cache.end()) ? it->second : "";
    }

private:
    std::mutex _cache_lock;
    std::map<std::string, std::string, std::less<>> _cache;
};

class HttpSession : public CppServer::HTTP::HTTPSession
{
public:
    using CppServer::HTTP::HTTPSession::HTTPSession;

protected:
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override
    {
        // Show HTTP request content
        std::cout << "Request method: " << request.method() << std::endl;
        std::cout << "Request URL: " << request.url() << std::endl;
        std::cout << "Request protocol: " << request.protocol() << std::endl;
        std::cout << "Request headers: " << request.headers() << std::endl;
        for (size_t i = 0; i < request.headers(); ++i)
        {
            auto header = request.header(i);
            std::cout << std::get<0>(header) << ": " << std::get<1>(header) << std::endl;
        }
        std::cout << "Request body:" << request.body_length() << std::endl;
        std::cout << request.body() << std::endl;

        // Process HTTP request methods
        if (request.method() == "HEAD")
        {
            // Fill and send the corresponding HTTP response
            auto& response = this->response();
            response.Clear();
            response.SetBegin(200);
            response.SetHeader("Content-Type", "text/html; charset=UTF-8");
            response.SetBodyLength(0);
            SendResponseAsync(response);
        }
        else if (request.method() == "GET")
        {
            // Get the cache value
            auto cache = Cache::GetInstance().GetCache(request.url());

            // Fill and send the corresponding HTTP response
            auto& response = this->response();
            response.Clear();
            response.SetBegin(200);
            response.SetHeader("Content-Type", "text/html; charset=UTF-8");
            response.SetBody(cache);
            SendResponseAsync(response);
        }
        else if (request.method() == "TRACE")
        {
            // Fill and send the corresponding HTTP response
            auto& response = this->response();
            response.Clear();
            response.SetBegin(200);
            response.SetHeader("Content-Type", "message/http");
            response.SetBody(request.cache());
            SendResponseAsync(response);
        }
        else
            std::cout << "Unsupported HTTP method: " << request.method() << std::endl;
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

class HttpServer : public CppServer::HTTP::HTTPServer
{
public:
    using CppServer::HTTP::HTTPServer::HTTPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(std::shared_ptr<CppServer::Asio::TCPServer> server) override
    {
        return std::make_shared<HttpSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // HTTP server port
    int port = 80;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "HTTP server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP server
    auto server = std::make_shared<HttpServer>(service, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
