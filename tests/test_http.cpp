//
// Created by Ivan Shynkarenka on 08.02.2019
//

#include "test.h"

#include "server/http/http_request.h"
#include "server/http/http_response.h"

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

TEST_CASE("HTTP response test", "[CppServer][HTTP]")
{
    // Create a new HTTP response
    HTTPResponse response;

    // Prepare HTTP response
    response.SetBegin(200);
    response.SetHeader("Accept-Ranges", "bytes");
    response.SetHeader("Content-Type", "text/html; charset=UTF-8");
    response.SetBody("test");

    // Check the HTTP response state
    REQUIRE(response.status() == 200);
    REQUIRE(response.status_phrase() == "OK");
    REQUIRE(response.protocol() == "HTTP/1.1");
    REQUIRE(response.headers() == 3);
    REQUIRE(std::get<0>(response.header(0)) == "Accept-Ranges");
    REQUIRE(std::get<1>(response.header(0)) == "bytes");
    REQUIRE(std::get<0>(response.header(1)) == "Content-Type");
    REQUIRE(std::get<1>(response.header(1)) == "text/html; charset=UTF-8");
    REQUIRE(std::get<0>(response.header(2)) == "Content-Length");
    REQUIRE(std::get<1>(response.header(2)) == "4");
    REQUIRE(response.body() == "test");
}
