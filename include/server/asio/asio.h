/*!
    \file asio.h
    \brief Asio C++ Library definition
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_H
#define CPPSERVER_ASIO_H

#include <iostream>

#if defined(__clang__)
#pragma clang system_header
#elif defined(__GNUC__)
#pragma GCC system_header
#elif defined(_MSC_VER)
//#pragma system_header
#endif

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

//! Socket buffer chunk
const size_t CHUNK = 8192;

//! Internet protocol
enum class InternetProtocol
{
    IPv4,               //!< Internet Protocol version 4
    IPv6                //!< Internet Protocol version 6
};

//! Stream output: Internet protocol
/*!
    \param stream - Output stream
    \param protocol - Internet protocol
    \return Output stream
*/
std::ostream& operator<<(std::ostream& stream, InternetProtocol protocol);

} // namespace Asio

} // namespace CppServer

#endif // CPPSERVER_ASIO_H
