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

#include "../../modules/asio/asio/include/asio.hpp"

//! Internet protocols
enum class InternetProtocol
{
    IPv4,               //!< Internet Protocol version 4
    IPv6                //!< Internet Protocol version 6
};

#endif // CPPSERVER_ASIO_H
