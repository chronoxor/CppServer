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

const size_t TCPClient::CHUNK;

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
      _connecting(false),
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

        // Shutdown the client socket
        _socket.shutdown(asio::ip::tcp::socket::shutdown_both);

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

    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the send buffer
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

    if (!IsConnected())
        return;

    uint8_t buffer[CHUNK];

    _reciving = true;
    auto self(this->shared_from_this());
    _socket.async_read_some(asio::buffer(buffer), [this, self, &buffer](std::error_code ec, std::size_t size)
    {
        _reciving = false;

        if (!IsConnected())
            return;

        // Received some data from the client
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

    uint8_t buffer[CHUNK];
    size_t size;

    {
        std::lock_guard<std::mutex> locker(_send_lock);

        // Fill the send buffer
        size = std::min(_send_buffer.size(), CHUNK);
        std::memcpy(buffer, _send_buffer.data(), size);
    }

    _sending = true;
    auto self(this->shared_from_this());
    asio::async_write(_socket, asio::buffer(buffer, size), [this, self](std::error_code ec, std::size_t size)
    {
        _sending = false;

        if (!IsConnected())
            return;

        bool resume = true;

        // Send some data to the client
        if (size > 0)
        {
            // Update statistic
            _bytes_sent += size;

            // Call the buffer sent handler
            onSent(size, _send_buffer.size());

            {
                std::lock_guard<std::mutex> locker(_send_lock);

                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    resume = false;
            }
        }

        // Try to send again if the session is valid
        if (!ec)
        {
            if (resume)
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
    std::lock_guard<std::mutex> locker(_send_lock);

    _recive_buffer.clear();
    _send_buffer.clear();
}

void TCPClient::SendError(std::error_code ec)
{
    // Skip Asio disconnect errors
    if ((ec == asio::error::connection_aborted) ||
        (ec == asio::error::connection_refused) ||
        (ec == asio::error::eof) ||
        (ec == asio::error::operation_aborted))
        return;

    onError(ec.value(), ec.category().name(), ec.message());
}

} // namespace Asio
} // namespace CppServer
