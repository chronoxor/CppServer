//
// Created by Ivan Shynkarenka on 26.05.2016.
//

#include "catch.hpp"

#include "template/function.h"

using namespace CppTemplate;

TEST_CASE("Template function", "[CppTemplate]")
{
    for (int i = 0; i < 10; ++i)
        REQUIRE(function(10) < 10);

    for (int i = 0; i < 100; ++i)
        REQUIRE(function(100) < 100);

    for (int i = 0; i < 1000; ++i)
        REQUIRE(function(1000) < 1000);
}
