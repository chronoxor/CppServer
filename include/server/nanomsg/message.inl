/*!
    \file message.inl
    \brief Nanomsg message inline implementation
    \author Ivan Shynkarenka
    \date 27.01.2017
    \copyright MIT License
*/

namespace CppServer {
namespace Nanomsg {

inline void Message::swap(Message& message) noexcept
{
    using std::swap;
    swap(_buffer, message._buffer);
    swap(_size, message._size);
    swap(_type, message._type);
}

inline void swap(Message& message1, Message& message2) noexcept
{
    message1.swap(message2);
}

} // namespace Nanomsg
} // namespace CppServer
