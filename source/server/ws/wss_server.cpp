/*!
    \file wss_server.cpp
    \brief WebSocket secure server implementation
    \author Ivan Shynkarenka
    \date 27.05.2019
    \copyright MIT License
*/

#include "server/ws/wss_server.h"

namespace CppServer {
namespace WS {

bool WSSServer::CloseAll(int status, const void* buffer, size_t size)
{
    std::scoped_lock locker(_ws_send_lock);

    PrepareSendFrame(WS_FIN | WS_CLOSE, false, buffer, size, status);
    if (!Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()))
        return false;

    return HTTPSServer::DisconnectAll();
}

bool WSSServer::CloseAll(int status, std::string_view text)
{
    std::scoped_lock locker(_ws_send_lock);

    PrepareSendFrame(WS_FIN | WS_CLOSE, false, text.data(), text.size(), status);
    if (!Multicast(_ws_send_buffer.data(), _ws_send_buffer.size()))
        return false;

    return HTTPSServer::DisconnectAll();
}

bool WSSServer::Multicast(const void* buffer, size_t size)
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
        auto wss_session = std::dynamic_pointer_cast<WSSSession>(session.second);
        if (wss_session)
        {
            std::scoped_lock ws_locker(wss_session->_ws_send_lock);

            if (wss_session->_ws_handshaked)
                wss_session->SendAsync(buffer, size);
        }
    }

    return true;
}

} // namespace WS
} // namespace CppServer
