/*!
    \file asio.h
    \brief Asio C++ Library definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_H
#define CPPSERVER_ASIO_H

#define ASIO_STANDALONE
#define ASIO_SEPARATE_COMPILATION

#include <asio.hpp>
#include <asio/ssl.hpp>

#if defined(_WIN32) || defined(_WIN64)
#undef Yield
#endif

namespace CppServer {

/*!
    \namespace CppServer::Asio
    \brief Asio definitions
*/
namespace Asio {

//! Internet protocol
enum class InternetProtocol
{
    IPv4,               //!< Internet Protocol version 4
    IPv6                //!< Internet Protocol version 6
};

} // namespace Asio

} // namespace CppServer

#endif // CPPSERVER_ASIO_H
