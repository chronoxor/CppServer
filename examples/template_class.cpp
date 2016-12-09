/*!
    \file template_class.cpp
    \brief Template class example
    \author Ivan Shynkarenka
    \date 26.05.2016
    \copyright MIT License
*/

#include "template/class.h"

#include <iostream>

int main(int argc, char** argv)
{
    CppTemplate::Template instance(10);

    std::cout << "instance.field() = " << instance.field() << std::endl;
    std::cout << "instance.Method(100) = " << instance.Method(100) << std::endl;
    std::cout << "CppTemplate::Template::StaticMethod(1000) = " << CppTemplate::Template::StaticMethod(1000) << std::endl;
    return 0;
}
