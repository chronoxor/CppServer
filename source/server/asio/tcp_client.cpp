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
      _connected(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _sending(false)
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
      _connected(false),
      _bytes_sent(0),
      _bytes_received(0),
      _reciving(false),
      _sending(false)
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
        // Connect the client socket
        _socket.async_connect(_endpoint, [this, self](std::error_code ec)
        {
            if (!ec)
            {
                // Put the socket into non-blocking mode
                _socket.non_blocking(true);

                // Set the socket keep-alive option
                asio::ip::tcp::socket::keep_alive keep_alive(true);
                _socket.set_option(keep_alive);

                // Reset statistic
                _bytes_sent = 0;
                _bytes_received = 0;

                // Update the connected flag
                _connected = true;

                // Call the client connected handler
                onConnected();

                // Try to receive something from the server
                TryReceive();
            }
            else
            {
                // Call the client disconnected handler
                onError(ec.value(), ec.category().name(), ec.message());
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

    // Fill the send buffer
    {
        std::lock_guard<std::mutex> locker(_send_lock);
        const uint8_t* bytes = (const uint8_t*)buffer;
        _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);
    }

    // Dispatch the send routine
    auto self(this->shared_from_this());
    _service->Dispatch([this, self]()
    {
        // Try to send the buffer
        TrySend();
    });

    return _send_buffer.size();
}

void TCPClient::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec)
    {
        _reciving = false;

        // Check for disconnect
        if (!IsConnected())
            return;

        // Receive some data from the server in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _socket.read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                // Update statistic
                _bytes_received += size;

                // Fill receive buffer
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call the buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

                // Erase the handled buffer
                _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
            }
        }

        // Try to receive again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            service()->Post([this, self]() { TryReceive(); });
        else
        {
            if (ec != asio::error::connection_reset)
                onError(ec.value(), ec.category().name(), ec.message());
            Disconnect(true);
        }
    });
}

void TCPClient::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Check for disconnect
        if (!IsConnected())
            return;

        // Send some data to the server in non blocking mode
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Update statistic
                _bytes_sent += size;

                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Call the buffer sent handler
                onSent(size, _send_buffer.size());

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    return;
            }
        }

        // Try to send again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            service()->Post([this, self]() { TrySend(); });
        else
        {
            if (ec != asio::error::connection_reset)
                onError(ec.value(), ec.category().name(), ec.message());
            Disconnect(true);
        }
    });
}

void TCPClient::ClearBuffers()
{
    std::lock_guard<std::mutex> locker(_send_lock);
    _recive_buffer.clear();
    _send_buffer.clear();
}

} // namespace Asio
} // namespace CppServer
