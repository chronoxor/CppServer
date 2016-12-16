/*!
    \file client.inl
    \brief TCP client inline implementation
    \author Ivan Shynkarenka
    \date 15.12.2016
    \copyright MIT License
*/

namespace CppServer {

inline TCPClient::TCPClient(const std::string& address, uint16_t port)
    : _id(CppCommon::UUID::Generate()),
      _service(),
      _socket(_service),
      _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port)),
      _started(false),
      _ñonnected(false),
      _reciving(false),
      _sending(false)
{
}

inline void TCPClient::Start()
{
    if (IsStarted())
        return;

    // Call client starting handler
    onStarting();

    // Start client thread
    _thread = std::thread([this]() { ClientLoop(); });

    // Update started flag
    _started = true;
}

inline void TCPClient::Stop()
{
    if (!IsStarted())
        return;

    // Call client stopping handler
    onStopping();

    // Update started flag
    _started = false;

    // Stop Asio service
    _service.stop();

    // Wait for client thread
    _thread.join();
}

inline void TCPClient::ClientLoop()
{
    // Call initialize thread handler
    onThreadInitialize();

    try
    {
        // Call client started handler
        onStarted();

        // Run Asio service in a loop with skipping errors
        while (_started)
        {
            asio::error_code ec;
            _service.run(ec);
            if (ec)
                onError(ec.value(), ec.category().name(), ec.message());
        }

        // Call client stopped handler
        onStopped();
    }
    catch (...)
    {
        fatality("TCP client thread terminated!");
    }

    // Call cleanup thread handler
    onThreadCleanup();
}

inline bool TCPClient::Connect()
{
    if (IsConnected() || !IsStarted())
        return false;

    // Post the connect routine
    _service.post([this]()
    {
        _socket.async_connect(_endpoint, [this](std::error_code ec)
        {
            if (!ec)
            {
                // Put the socket into non-blocking mode
                _socket.non_blocking(true);

                // Update connected flag
                _ñonnected = true;

                // Call client connected handler
                onConnected();

                // Try to receive something from the server
                TryReceive();
            }
            else
            {
                onError(ec.value(), ec.category().name(), ec.message());

                // Call the client disconnected handler
                onDisconnected();
            }
        });
    });

    return true;
}

inline bool TCPClient::Disconnect()
{
    if (!IsConnected() || !IsStarted())
        return false;

    // Post the disconnect routine
    _service.post([this]()
    {
        // Update connected flag
        _ñonnected = false;

        // Close the client socket
        _socket.close();

        // Call the client disconnected handler
        onDisconnected();
    });

    return true;
}

inline size_t TCPClient::Send(const void* buffer, size_t size)
{
    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Post the send routine
    _service.post([this]()
    {
        // Try to send the buffer if it is the first buffer to send
        if (!_sending)
            TrySend();
    });

    return size;
}

inline void TCPClient::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    _socket.async_wait(asio::ip::tcp::socket::wait_read, [this](std::error_code ec)
    {
        _reciving = false;

        // Perform receive some data from the server in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _socket.read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

                // Erase handled buffer
                _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
            }
        }

        // Try to receive again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
            Disconnect();
    });
}

inline void TCPClient::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    _socket.async_wait(asio::ip::tcp::socket::wait_write, [this](std::error_code ec)
    {
        _sending = false;

        // Perform send some data to the server in non blocking mode
        size_t sent = 0;
        size_t pending = 0;
        bool repeat = true;
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            std::error_code ec;
            size_t size = _socket.write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Fill sent handler parameters
                sent = size;
                pending = _send_buffer.size();

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    repeat = false;
            }
        }

        // Call buffer sent handler
        if (sent > 0)
            onSent(sent, pending);

        // Stop send loop if there is nothing to send
        if (!repeat)
            return;

        // Try to send again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
            Disconnect();
    });
}

} // namespace CppServer
