//
// Created by Ivan Shynkarenka on 08.02.2019
//

#include "test.h"

#include "server/http/http_request.h"

using namespace CppServer::HTTP;

TEST_CASE("HTTP request test", "[CppServer][HTTP]")
{
    // Create a new HTTP request
    HTTPRequest request;

    // Prepare HTTP request
    request.SetBegin("PUT", "/");
    request.SetHeader("Host", "example.com");
    request.SetHeader("User-Agent", "Mozilla/5.0");
    request.SetBody("test");

    // Check the HTTP request state
    REQUIRE(request.method() == "PUT");
    REQUIRE(request.url() == "/");
    REQUIRE(request.protocol() == "HTTP/1.1");
    REQUIRE(request.headers() == 3);
    REQUIRE(std::get<0>(request.header(0)) == "Host");
    REQUIRE(std::get<1>(request.header(0)) == "example.com");
    REQUIRE(std::get<0>(request.header(1)) == "User-Agent");
    REQUIRE(std::get<1>(request.header(1)) == "Mozilla/5.0");
    REQUIRE(std::get<0>(request.header(2)) == "Content-Length");
    REQUIRE(std::get<1>(request.header(2)) == "4");
    REQUIRE(request.body() == "test");
}
