/*!
    \file tcp_client.cpp
    \brief TCP client implementation
    \author Ivan Shynkarenka
    \date 15.12.2016
    \copyright MIT License
*/

#include "server/asio/tcp_client.h"

namespace CppServer {
namespace Asio {

TCPClient::TCPClient(std::shared_ptr<Service> service, const std::string& address, int port)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port)),
      _socket(*_service->service()),
      _connecting(false),
      _connected(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _recive_buffer(CHUNK + 1),
      _sending(false),
      _send_buffer_flush_offset(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

TCPClient::TCPClient(std::shared_ptr<Service> service, const asio::ip::tcp::endpoint& endpoint)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _endpoint(endpoint),
      _socket(*_service->service()),
      _connecting(false),
      _connected(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _recive_buffer(CHUNK + 1),
      _sending(false),
      _send_buffer_flush_offset(0)
{
    assert((service != nullptr) && "ASIO service is invalid!");
    if (service == nullptr)
        throw CppCommon::ArgumentException("ASIO service is invalid!");
}

bool TCPClient::Connect()
{
    if (IsConnected())
        return false;

    // Post the connect routine
    auto self(this->shared_from_this());
    _service->service()->post([this, self]()
    {
        if (IsConnected() || _connecting)
            return;

        // Connect the client socket
        _connecting = true;
        _socket.async_connect(_endpoint, [this, self](std::error_code ec)
        {
            _connecting = false;

            if (!ec)
            {
                // Reset statistic
                _bytes_sent = 0;
                _bytes_received = 0;

                // Update the connected flag
                _connected = true;

                // Call the client connected handler
                onConnected();

                // Call the empty send buffer handler
                onEmpty();

                // Try to receive something from the server
                TryReceive();
            }
            else
            {
                // Call the client disconnected handler
                SendError(ec);
                onDisconnected();
            }
        });
    });

    return true;
}

bool TCPClient::Disconnect(bool dispatch)
{
    if (!IsConnected())
        return false;

    auto self(this->shared_from_this());
    auto disconnect = [this, self]()
    {
        if (!IsConnected())
            return;

        // Close the client socket
        _socket.close();

        // Clear receive/send buffers
        ClearBuffers();

        // Update the connected flag
        _connected = false;

        // Call the client disconnected handler
        onDisconnected();
    };

    // Dispatch or post the disconnect routine
    if (dispatch)
        _service->Dispatch(disconnect);
    else
        _service->Post(disconnect);

    return true;
}

bool TCPClient::Reconnect()
{
    if (!Disconnect())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return Connect();
}

size_t TCPClient::Send(const void* buffer, size_t size)
{
    assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
    assert((size > 0) && "Buffer size should be greater than zero!");
    if ((buffer == nullptr) || (size == 0))
        return 0;

    if (!IsConnected())
        return 0;

    size_t result;
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the main send buffer
        const uint8_t* bytes = (const uint8_t*)buffer;
        _send_buffer_main.insert(_send_buffer_main.end(), bytes, bytes + size);
        result = _send_buffer_main.size();
    }

    // Dispatch the send routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        // Try to send the main buffer
        TrySend();
    });

    return result;
}

void TCPClient::TryReceive()
{
    if (_reciving)
        return;

    if (!IsConnected())
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_read_some(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), [this, self](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsConnected())
            return;

        // Received some data from the client
        if (size > 0)
        {
            // Update statistic
            _bytes_received += size;

            // If the receive buffer is full increase its size
            if (_recive_buffer.size() == size)
                _recive_buffer.resize(2 * size);

            // Call the buffer received handler
            onReceived(_recive_buffer.data(), size);
        }

        // Try to receive again if the session is valid
        if (!ec)
            TryReceive();
        else
        {
            SendError(ec);
            Disconnect(true);
        }
    });
}

void TCPClient::TrySend()
{
    if (_sending)
        return;

    if (!IsConnected())
        return;

    // Swap send buffers
    if (_send_buffer_flush.empty())
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Swap flush and main buffers
        _send_buffer_flush.swap(_send_buffer_main);
        _send_buffer_flush_offset = 0;
    }

    // Check if the flush buffer is empty
    if (_send_buffer_flush.empty())
    {
        // Call the empty send buffer handler
        onEmpty();
        return;
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_socket, asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), [this, self](std::error_code ec, std::size_t size)
    {
        _sending = false;

        if (!IsConnected())
            return;

        // Send some data to the client
        if (size > 0)
        {
            // Update statistic
            _bytes_sent += size;

            // Increase the flush buffer offset
            _send_buffer_flush_offset += size;

            // Successfully send the whole flush buffer
            if (_send_buffer_flush_offset == _send_buffer_flush.size())
            {
                // Clear the flush buffer
                _send_buffer_flush.clear();
                _send_buffer_flush_offset = 0;
            }

            // Call the buffer sent handler
            onSent(size, _send_buffer_flush.size() - _send_buffer_flush_offset);
        }

        // Try to send again if the session is valid
        if (!ec)
        {
            TrySend();
        }
        else
        {
            SendError(ec);
            Disconnect(true);
        }
    });
}

void TCPClient::ClearBuffers()
{
    // Clear send buffers
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        _send_buffer_main.clear();
        _send_buffer_flush.clear();
        _send_buffer_flush_offset = 0;
    }
}

void TCPClient::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::connection_reset) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
