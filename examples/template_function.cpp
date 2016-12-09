/*!
    \file template_function.cpp
    \brief Template function example
    \author Ivan Shynkarenka
    \date 26.05.2016
    \copyright MIT License
*/

#include "template/function.h"

#include <iostream>

int main(int argc, char** argv)
{
    std::cout << "CppTemplate::function(10) = " << CppTemplate::function(10) << std::endl;
    std::cout << "CppTemplate::function(100) = " << CppTemplate::function(100) << std::endl;
    std::cout << "CppTemplate::function(1000) = " << CppTemplate::function(1000) << std::endl;
    return 0;
}
