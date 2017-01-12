/*!
    \file websocket.h
    \brief WebSocket C++ Library definition
    \author Ivan Shynkarenka
    \date 06.01.2016
    \copyright MIT License
*/

#ifndef CPPSERVER_WEBSOCKET_H
#define CPPSERVER_WEBSOCKET_H

#include "asio.hpp"

#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_THREAD_

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>

namespace CppServer {
namespace Asio {

//! WebSocket client core
typedef websocketpp::client<websocketpp::config::asio> WebSocketClientCore;
//! WebSocket server core
typedef websocketpp::server<websocketpp::config::asio> WebSocketServerCore;
//! WebSocket connection
typedef websocketpp::connection<websocketpp::config::asio> WebSocketConnection;
//! WebSocket message
typedef WebSocketConnection::message_ptr WebSocketMessage;

} // namespace Asio
} // namespace CppServer

#endif // CPPSERVER_WEBSOCKET_H
