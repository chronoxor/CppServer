/*!
    \file asio.h
    \brief Asio C++ Library definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_H
#define CPPSERVER_ASIO_H

#define ASIO_STANDALONE 1
#define ASIO_SEPARATE_COMPILATION 1

#include "../../../modules/asio/asio/include/asio.hpp"

#if defined(_WIN32) || defined(_WIN64)
#undef Yield
#endif

namespace CppServer {

/*!
    \namespace Asio
    \brief Asio definitions
*/
namespace Asio {

//! Internet protocols
enum class InternetProtocol
{
    IPv4,               //!< Internet Protocol version 4
    IPv6                //!< Internet Protocol version 6
};

} // namespace Asio

} // namespace CppServer

#endif // CPPSERVER_ASIO_H
