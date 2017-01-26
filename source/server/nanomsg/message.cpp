/*!
    \file message.cpp
    \brief Nanomsg message implementation
    \author Ivan Shynkarenka
    \date 27.01.2017
    \copyright MIT License
*/

#include "server/nanomsg/message.h"

#include "errors/exceptions.h"
#include "errors/fatal.h"
#include "string/format.h"

#include <cassert>

namespace CppServer {
namespace Nanomsg {

Message::Message() : _buffer(nullptr), _size(0), _type(0)
{
}

Message::Message(size_t size, int type) : _buffer(nullptr), _size(0), _type(type)
{
    assert((size > 0) && "Message size should be greater than zero!");
    _buffer = (uint8_t*)nn_allocmsg(size, type);
    if (_buffer == nullptr)
        throwex CppCommon::SystemException("Failed to allocate a memory buffer of {} bytes for the nanomsg message! Nanomsg error: {}"_format(size, nn_strerror(errno)));
    _size = size;
}

Message::Message(const void* data, size_t size, int type) : Message(size, type)
{
    std::memcpy(_buffer, data, size);
}

Message::Message(const Message& message) : Message(message.buffer(), message.size(), message.type())
{
}

Message::~Message()
{
    if (_buffer != nullptr)
    {
        int result = nn_freemsg(_buffer);
        if (result != 0)
            fatality(CppCommon::SystemException("Failed to free memory of the nanomsg message!"));
        _buffer = nullptr;
    }
}

Message& Message::operator=(const Message& message)
{
    Message temp(message);
    *this = std::move(temp);
    return *this;
}

void Message::Reallocate(size_t size)
{
    assert((size > 0) && "Message size should be greater than zero!");
    if (_buffer != nullptr)
    {
        _buffer = (uint8_t*)nn_reallocmsg(_buffer, size);
        if (_buffer == nullptr)
            throwex CppCommon::SystemException("Failed to reallocate a memory buffer of {} bytes for the nanomsg message! Nanomsg error: {}"_format(size, nn_strerror(errno)));
    }
    else
    {
        _buffer = (uint8_t*)nn_allocmsg(size, _type);
        if (_buffer == nullptr)
            throwex CppCommon::SystemException("Failed to allocate a memory buffer of {} bytes for the nanomsg message! Nanomsg error: {}"_format(size, nn_strerror(errno)));
    }
    _size = size;
}

} // namespace Nanomsg
} // namespace CppServer
