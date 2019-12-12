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

    PrepareSendFrame(WS_FIN | WS_CLOSE, false, nullptr, 0, status);
    if (!Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()))
        return false;

    return HTTPServer::DisconnectAll();
}

bool WSServer::Multicast(const void* buffer, size_t size)
{
    if (!IsStarted())
        return false;

    if (size == 0)
        return true;

    assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
    if (buffer == nullptr)
        return false;

    std::shared_lock<std::shared_mutex> locker(_sessions_lock);

    // Multicast all WebSocket sessions
    for (auto& session : _sessions)
    {
        auto ws_session = std::dynamic_pointer_cast<WSSession>(session.second);
        if (ws_session)
        {
            std::scoped_lock ws_locker(ws_session->_ws_send_lock);

            if (ws_session->_ws_handshaked)
                ws_session->SendAsync(buffer, size);
        }
    }

    return true;
}

} // namespace WS
} // namespace CppServer
