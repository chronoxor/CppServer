/*!
    \file memory.h
    \brief Asio memory manager definition
    \author Ivan Shynkarenka
    \date 16.02.2018
    \copyright MIT License
*/

#ifndef CPPSERVER_ASIO_MEMORY_H
#define CPPSERVER_ASIO_MEMORY_H

#include <memory>

namespace CppServer {
namespace Asio {

//! Asio handler storage
/*!
    Class to manage the memory to be used for handler-based custom allocation.
    It contains a single block of memory which may be returned for allocation
    requests. If the memory is in use when an allocation request is made, the
    allocator delegates allocation to the global heap.

    Not thread-safe.
*/
class HandlerStorage
{
public:
    HandlerStorage() noexcept : _in_use(false) {}
    HandlerStorage(const HandlerStorage&) = delete;
    HandlerStorage(HandlerStorage&&) = delete;
    ~HandlerStorage() noexcept = default;

    HandlerStorage& operator=(const HandlerStorage&) = delete;
    HandlerStorage& operator=(HandlerStorage&&) = delete;

    //! Allocate memory buffer
    /*!
        \param size - Size of allocated block in bytes
        \return Pointer to the allocated buffer
    */
    void* allocate(size_t size);
    //! Deallocate memory buffer
    /*!
        \param ptr - Pointer to the allocated buffer
    */
    void deallocate(void* ptr);

private:
    // Whether the handler-based custom allocation storage has been used
    bool _in_use;
    // Storage space used for handler-based custom memory allocation
    typename std::aligned_storage<1024>::type _storage;
};

//! Asio handler allocator
/*!
    The allocator to be associated with the handler objects. This allocator only
    needs to satisfy the C++11 minimal allocator requirements.

    Not thread-safe.
*/
template <typename T>
class HandlerAllocator
{
    template <typename>
    friend class HandlerAllocator;

public:
    //! Element type
    typedef T value_type;
    //! Pointer to element
    typedef T* pointer;
    //! Reference to element
    typedef T& reference;
    //! Pointer to constant element
    typedef const T* const_pointer;
    //! Reference to constant element
    typedef const T& const_reference;
    //! Quantities of elements
    typedef size_t size_type;
    //! Difference between two pointers
    typedef ptrdiff_t difference_type;

    //! Initialize allocator with a given memory storage
    /*!
        \param storage - Memory storage
    */
    explicit HandlerAllocator(HandlerStorage& storage) noexcept : _storage(storage) {}
    template <typename U>
    HandlerAllocator(const HandlerAllocator<U>& alloc) noexcept : _storage(alloc._storage) {}
    HandlerAllocator(const HandlerAllocator& alloc) noexcept : _storage(alloc._storage) {}
    HandlerAllocator(HandlerAllocator&&) noexcept = default;
    ~HandlerAllocator() noexcept = default;

    template <typename U>
    HandlerAllocator& operator=(const HandlerAllocator<U>& alloc) noexcept
    { _storage = alloc._storage; return *this; }
    HandlerAllocator& operator=(const HandlerAllocator& alloc) noexcept
    { _storage = alloc._storage; return *this; }
    HandlerAllocator& operator=(HandlerAllocator&&) noexcept = default;

    //! Allocate a block of storage suitable to contain the given count of elements
    /*!
        \param num - Number of elements to be allocated
        \param hint - Allocation hint (default is 0)
        \return A pointer to the initial element in the block of storage
    */
    pointer allocate(size_type num, const void* hint = 0) { return (pointer)_storage.allocate(num * sizeof(T)); }
    //! Release a block of storage previously allocated
    /*!
        \param ptr - Pointer to a block of storage
        \param num - Number of releasing elements
    */
    void deallocate(pointer ptr, size_type num) { return _storage.deallocate(ptr); }

private:
    // The underlying handler storage
    HandlerStorage& _storage;
};

//! Asio allocate handler wrapper
/*!
    Wrapper class template for handler objects to allow handler memory
    allocation to be customised. The allocator_type type and get_allocator()
    member function are used by the asynchronous operations to obtain the
    allocator. Calls to operator() are forwarded to the encapsulated handler.

    Not thread-safe.
*/
template <typename THandler>
class AllocateHandler
{
public:
    //! Allocator type
    typedef HandlerAllocator<THandler> allocator_type;

    //! Initialize allocate handler wrapper with a given memory storage and handler
    /*!
        \param storage - Memory storage
        \param handler - Handler to allocate
    */
    AllocateHandler(HandlerStorage& storage, THandler handler) noexcept : _storage(storage), _handler(handler) {}
    AllocateHandler(const AllocateHandler&) noexcept = default;
    AllocateHandler(AllocateHandler&&) noexcept = default;
    ~AllocateHandler() noexcept = default;

    AllocateHandler& operator=(const AllocateHandler&) noexcept = default;
    AllocateHandler& operator=(AllocateHandler&&) noexcept = default;

    //! Get the handler allocator
    allocator_type get_allocator() const noexcept { return allocator_type(_storage); }

    //! Wrap the handler
    template <typename ...Args>
    void operator()(Args&&... args) { _handler(std::forward<Args>(args)...); }

private:
    HandlerStorage& _storage;
    THandler _handler;
};

//! Helper function to wrap a handler object to add custom allocation
/*!
    \param storage - Memory storage
    \param handler - Handler to allocate
*/
template <typename THandler>
AllocateHandler<THandler> make_alloc_handler(HandlerStorage& storage, THandler handler);

} // namespace Asio
} // namespace CppServer

#include "memory.inl"

#endif // CPPSERVER_ASIO_MEMORY_H
