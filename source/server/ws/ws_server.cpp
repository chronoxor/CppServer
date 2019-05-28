/*!
    \file ws_server.cpp
    \brief WebSocket server implementation
    \author Ivan Shynkarenka
    \date 27.05.2019
    \copyright MIT License
*/

#include "server/ws/ws_server.h"

namespace CppServer {
namespace WS {

bool WSServer::CloseAll(int status)
{
    std::scoped_lock locker(_ws_send_lock);

    PrepareSendFrame(WS_FIN | WS_CLOSE, nullptr, 0, status);
    if (!HTTPServer::Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()))
        return false;

    return HTTPServer::DisconnectAll();
}

} // namespace WS
} // namespace CppServer
