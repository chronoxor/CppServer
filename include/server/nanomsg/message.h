/*!
    \file message.h
    \brief Nanomsg message definition
    \author Ivan Shynkarenka
    \date 27.01.2017
    \copyright MIT License
*/

#ifndef CPPSERVER_NANOMSG_MESSAGE_H
#define CPPSERVER_NANOMSG_MESSAGE_H

#include "nanomsg.h"

#include "errors/exceptions.h"

#include <iostream>
#include <string>

namespace CppServer {
namespace Nanomsg {

//! Nanomsg message
/*!
    Nanomsg message wraps message buffer and used to send or receive data
    using Nanomsg library API.

    Thread-safe.
*/
class Message
{
    friend class Socket;

public:
    //! Create an empty message
    Message();
    //! Allocate memory for a new message
    /*!
        Allocate type parameter specifies type of allocation mechanism to use.
        Zero is the default one, however, individual transport mechanisms may
        define their own allocation mechanisms, such as allocating in shared
        memory or allocating a memory block pinned down to a physical memory
        address. Such allocation, when used with the transport that defines
        them, should be more efficient than the default allocation mechanism.

        \param size - Message size
        \param type - Allocate type (default is 0)
    */
    explicit Message(size_t size, int type = 0);
    //! Create a new message based on the given buffer
    /*!
        \param data - Message data
        \param size - Message size
        \param type - Allocate type (default is 0)
    */
    explicit Message(const void* data, size_t size, int type = 0);
    Message(const Message& message);
    Message(Message&&) = default;
    ~Message();

    Message& operator=(const Message& message);
    Message& operator=(Message&&) = default;

    //! Check if the message is valid
    explicit operator bool() const { return (_buffer == nullptr); }

    //! Get the message buffer
    uint8_t* buffer() noexcept { return _buffer; }
    //! Get the constant message buffer
    const uint8_t* buffer() const noexcept { return _buffer; }
    //! Get the message size
    size_t size() const noexcept { return _size; }
    //! Get the message type
    int type() const noexcept { return _type; }

    //! Clear the message buffer
    void Clear();

    //! Reallocate the message size
    /*!
        \param size - New message size
    */
    void Reallocate(size_t size);

    //! Convert the current message to a string
    std::string string() const { return std::string((const char*)_buffer, _size); }

    //! Output instance into the given output stream
    friend std::ostream& operator<<(std::ostream& os, const Message& instance)
    { os << instance.string(); return os; }

    //! Swap two instances
    void swap(Message& message) noexcept;
    friend void swap(Message& message1, Message& message2) noexcept;

private:
    uint8_t* _buffer;
    size_t _size;
    int _type;
};

} // namespace Nanomsg
} // namespace CppServer

#include "message.inl"

#endif // CPPSERVER_NANOMSG_MESSAGE_H
