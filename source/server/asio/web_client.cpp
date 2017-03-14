/*!
    \file web_client.cpp
    \brief Web client implementation
    \author Ivan Shynkarenka
    \date 14.03.2017
    \copyright MIT License
*/

#include "server/asio/web_client.h"

namespace CppServer {
namespace Asio {

WebClient::WebClient(std::shared_ptr<Service> service)
    : _service(service),
      _settings(std::make_shared<restbed::Settings>()),
      _ssl_settings(std::make_shared<restbed::SSLSettings>())
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

const std::shared_ptr<restbed::Response> WebClient::Send(const std::shared_ptr<restbed::Request>& request)
{
    return restbed::Http::sync(request, _settings);
}

std::future<std::shared_ptr<restbed::Response>> WebClient::SendAsync(const std::shared_ptr<restbed::Request>& request, const std::function<void (const std::shared_ptr<restbed::Request>&, const std::shared_ptr<restbed::Response>&)>& callback)
{
    return restbed::Http::async(request, callback, _settings);
}

bool WebClient::IsOpened(const std::shared_ptr<restbed::Request>& request)
{
    return restbed::Http::is_open(request);
}

void WebClient::Close(const std::shared_ptr<restbed::Request>& request)
{
    restbed::Http::close(request);
}

std::vector<uint8_t> WebClient::Fetch(const std::shared_ptr<restbed::Response>& response, const size_t size)
{
    return restbed::Http::fetch(size, response);
}

std::vector<uint8_t> WebClient::Fetch(const std::shared_ptr<restbed::Response>& response, const std::string& delimiter)
{
    return restbed::Http::fetch(delimiter, response);
}

std::vector<uint8_t> WebClient::Convert(const std::shared_ptr<restbed::Request>& request)
{
    return restbed::Http::to_bytes(request);
}

std::vector<uint8_t> WebClient::Convert(const std::shared_ptr<restbed::Response>& response)
{
    return restbed::Http::to_bytes(response);
}

} // namespace Asio
} // namespace CppServer
