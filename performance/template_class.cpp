//
// Created by Ivan Shynkarenka on 26.05.2016.
//

#include "benchmark/cppbenchmark.h"

#include "template/class.h"

using namespace CppTemplate;

BENCHMARK("Template", 10000000)
{
    Template instance(10);

    instance.Method(100);
    instance.StaticMethod(1000);
}

BENCHMARK_MAIN()
