/*!
    \file ssl_client.inl
    \brief SSL client inline implementation
    \author Ivan Shynkarenka
    \date 01.01.2017
    \copyright MIT License
*/

namespace CppServer {
namespace Asio {

inline SSLClient::SSLClient(std::shared_ptr<Service> service, asio::ssl::context& context, const std::string& address, int port)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _context(context),
      _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port)),
      _stream(std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(_service->service(), _context)),
      _connected(false),
      _handshaked(false),
      _reciving(false),
      _sending(false)
{
}

inline SSLClient::SSLClient(std::shared_ptr<Service> service, asio::ssl::context& context, const asio::ip::tcp::endpoint& endpoint)
    : _id(CppCommon::UUID::Generate()),
      _service(service),
      _context(context),
      _endpoint(endpoint),
	  _stream(std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(_service->service(), _context)),
      _connected(false),
      _handshaked(false),
      _reciving(false),
      _sending(false)
{
}

inline bool SSLClient::Connect()
{
    if (!_service->IsStarted())
        return false;

    if (IsConnected())
        return false;

    // Post the connect routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Connect the client socket
        socket().async_connect(_endpoint, [this, self](std::error_code ec)
        {
            if (!ec)
            {
                // Put the socket into non-blocking mode
                socket().non_blocking(true);

                // Set the socket keep-alive option
                asio::ip::tcp::socket::keep_alive keep_alive(true);
                socket().set_option(keep_alive);

                // Update the connected flag
                _connected = true;

                // Call the client connected handler
                onConnected();

                // Perform SSL handshake
                _stream->async_handshake(asio::ssl::stream_base::client, [this, self](std::error_code ec)
                {
                    if (!ec)
                    {
                        // Update the handshaked flag
                        _handshaked = true;

                        // Call the client handshaked handler
                        onHandshaked();

                        // Try to receive something from the server
                        TryReceive();
                    }
                    else
                    {
                        // Disconnect on in case of the bad handshake
                        onError(ec.value(), ec.category().name(), ec.message());
                        Disconnect();
                    }
                });
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

inline bool SSLClient::Disconnect()
{
    if (!IsConnected())
        return false;

    // Post the disconnect routine
    auto self(this->shared_from_this());
    _service->service().post([this, self]()
    {
        // Update the handshaked flag
        _handshaked = false;

        // Update the connected flag
        _connected = false;

        // Clear receive/send buffers
        _recive_buffer.clear();
        {
            std::lock_guard<std::mutex> locker(_send_lock);
            _send_buffer.clear();
        }

        // Reset the client stream
		_stream = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(_service->service(), _context);

        // Call the client disconnected handler
        onDisconnected();
    });

    return true;
}

inline size_t SSLClient::Send(const void* buffer, size_t size)
{
    if (!IsHandshaked())
        return 0;

    std::lock_guard<std::mutex> locker(_send_lock);

    const uint8_t* bytes = (const uint8_t*)buffer;
    _send_buffer.insert(_send_buffer.end(), bytes, bytes + size);

    // Dispatch the send routine
    auto self(this->shared_from_this());
    _service->service().dispatch([this, self]()
    {
        // Try to send the buffer if it is the first buffer to send
        if (!_sending)
            TrySend();
    });

    return _send_buffer.size();
}

inline void SSLClient::TryReceive()
{
    if (_reciving)
        return;

    _reciving = true;
    auto self(this->shared_from_this());
    socket().async_wait(asio::ip::tcp::socket::wait_read, [this, self](std::error_code ec)
    {
        _reciving = false;

        // Receive some data from the server in non blocking mode
        if (!ec)
        {
            uint8_t buffer[CHUNK];
            size_t size = _stream->read_some(asio::buffer(buffer), ec);
            if (size > 0)
            {
                _recive_buffer.insert(_recive_buffer.end(), buffer, buffer + size);

                // Call the buffer received handler
                size_t handled = onReceived(_recive_buffer.data(), _recive_buffer.size());

                // Erase the handled buffer
                _recive_buffer.erase(_recive_buffer.begin(), _recive_buffer.begin() + handled);
            }
        }

        // Check for disconnect
        if (!IsConnected())
            return;

        // Try to receive again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TryReceive();
        else
        {
            if (ec != asio::error::eof)
                onError(ec.value(), ec.category().name(), ec.message());
            Disconnect();
        }
    });
}

inline void SSLClient::TrySend()
{
    if (_sending)
        return;

    _sending = true;
    auto self(this->shared_from_this());
    socket().async_wait(asio::ip::tcp::socket::wait_write, [this, self](std::error_code ec)
    {
        _sending = false;

        // Send some data to the server in non blocking mode
        if (!ec)
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            size_t size = _stream->write_some(asio::buffer(_send_buffer), ec);
            if (size > 0)
            {
                // Erase the sent buffer
                _send_buffer.erase(_send_buffer.begin(), _send_buffer.begin() + size);

                // Call the buffer sent handler
                onSent(size, _send_buffer.size());

                // Stop sending if the send buffer is empty
                if (_send_buffer.empty())
                    return;
            }
        }

        // Check for disconnect
        if (!IsConnected())
            return;

        // Try to send again if the client is valid
        if (!ec || (ec == asio::error::would_block))
            TrySend();
        else
        {
            onError(ec.value(), ec.category().name(), ec.message());
            Disconnect();
        }
    });
}

} // namespace Asio
} // namespace CppServer
