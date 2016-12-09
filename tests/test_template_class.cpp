//
// Created by Ivan Shynkarenka on 26.05.2016.
//

#include "catch.hpp"

#include "template/class.h"

using namespace CppTemplate;

TEST_CASE("Template class", "[CppTemplate]")
{
    Template instance(0);

    REQUIRE(instance.field() == 0);

    for (int i = 0; i < 1000; ++i)
        REQUIRE(instance.Method(1000) < 1000);

    REQUIRE(Template::StaticMethod(1000) == 1000);
}
