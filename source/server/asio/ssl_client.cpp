/*!
    \file ssl_client.cpp
    \brief SSL client implementation
    \author Ivan Shynkarenka
    \date 01.01.2017
    \copyright MIT License
*/

#include "server/asio/ssl_client.h"

#include <mutex>
#include <vector>

namespace CppServer {
namespace Asio {

//! @cond INTERNALS

class SSLClient::Impl : public std::enable_shared_from_this<SSLClient::Impl>
{
public:
    Impl(const CppCommon::UUID& id, std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port)
        : _id(id),
          _service(service),
          _context(context),
          _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), port)),
          _stream(_service->service(), *_context),
          _connected(false),
          _handshaked(false),
          _total_received(0),
          _total_sent(0),
          _reciving(false),
          _sending(false)
    {
    }

    Impl(const CppCommon::UUID& id, std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint)
        : _id(id),
          _service(service),
          _context(context),
          _endpoint(endpoint),
          _stream(_service->service(), *_context),
          _connected(false),
          _handshaked(false),
          _total_received(0),
          _total_sent(0),
          _reciving(false),
          _sending(false)
    {
    }

    Impl(const Impl&) = delete;
    Impl(Impl&&) = default;
    ~Impl() = default;

    Impl& operator=(const Impl&) = delete;
    Impl& operator=(Impl&&) = default;

    const CppCommon::UUID& id() const noexcept { return _id; }

    std::shared_ptr<Service>& service() noexcept { return _service; }
    std::shared_ptr<asio::ssl::context>& context() noexcept { return _context; }
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    asio::ssl::stream<asio::ip::tcp::socket>& stream() noexcept { return _stream; }
    asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& socket() noexcept { return _stream.lowest_layer(); }

    size_t& total_received() noexcept { return _total_received; }
    size_t& total_sent() noexcept { return _total_sent; }

    bool IsConnected() const noexcept { return _connected; }
    bool IsHandshaked() const noexcept { return _handshaked; }

    bool Connect(std::shared_ptr<SSLClient> client)
    {
        _client = client;

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

                    // Reset statistic
                    _total_received = 0;
                    _total_sent = 0;

                    // Update the connected flag
                    _connected = true;

                    // Call the client connected handler
                    onConnected();

                    // Perform SSL handshake
                    _stream.async_handshake(asio::ssl::stream_base::client, [this, self](std::error_code ec)
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
                            Disconnect(true);
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

    bool Disconnect(bool dispatch)
    {
        if (!IsConnected())
            return false;

        // Post the disconnect routine
        auto self(this->shared_from_this());
        auto disconnect = [this, self]()
        {
            // Shutdown the client stream
            _stream.async_shutdown([this, self](std::error_code ec)
            {
                // Close the client socket
                socket().close();

                // Clear receive/send buffers
                ClearBuffers();

                // Update the handshaked flag
                _handshaked = false;

                // Update the connected flag
                _connected = false;

                // Call the client reset handler
                onReset();

                // Call the client disconnected handler
                onDisconnected();
            });
        };

        // Dispatch or post the disconnect routine
        if (dispatch)
            _service->Dispatch(disconnect);
        else
            _service->Post(disconnect);

        return true;
    }

    size_t Send(const void* buffer, size_t size)
    {
        assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
        assert((size > 0) && "Buffer size should be greater than zero!");
        if ((buffer == nullptr) || (size == 0))
            return 0;

        if (!IsHandshaked())
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
            // Try to send the buffer if it is the first buffer to send
            if (!_sending)
                TrySend();
        });

        return _send_buffer.size();
    }

protected:
    void onConnected() { _client->onConnected(); }
    void onHandshaked() { _client->onHandshaked(); }
    void onDisconnected() { _client->onDisconnected(); }
    void onReset() { _client->onReset(); }
    size_t onReceived(const void* buffer, size_t size) { return _client->onReceived(buffer, size); }
    void onSent(size_t sent, size_t pending) { _client->onSent(sent, pending); }
    void onError(int error, const std::string& category, const std::string& message) { _client->onError(error, category, message); }

private:
    // Client Id
    CppCommon::UUID _id;
    // SSL client
    std::shared_ptr<SSLClient> _client;
    // Asio service
    std::shared_ptr<Service> _service;
    // Server SSL context, endpoint & client stream
    std::shared_ptr<asio::ssl::context> _context;
    asio::ip::tcp::endpoint _endpoint;
    asio::ssl::stream<asio::ip::tcp::socket> _stream;
    std::atomic<bool> _connected;
    std::atomic<bool> _handshaked;
    // Client statistic
    size_t _total_received;
    size_t _total_sent;
    // Receive & send buffers
    std::mutex _send_lock;
    std::vector<uint8_t> _recive_buffer;
    std::vector<uint8_t> _send_buffer;
    bool _reciving;
    bool _sending;

    static const size_t CHUNK = 8192;

    void TryReceive()
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
                size_t size = _stream.read_some(asio::buffer(buffer), ec);
                if (size > 0)
                {
                    // Update statistic
                    _total_received += size;

                    // Fill receive buffer
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
                onError(ec.value(), ec.category().name(), ec.message());
                Disconnect(true);
            }
        });
    }

    void TrySend()
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

                size_t size = _stream.write_some(asio::buffer(_send_buffer), ec);
                if (size > 0)
                {
                    // Update statistic
                    _total_sent += size;

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
                Disconnect(true);
            }
        });
    }

    void ClearBuffers()
    {
        std::lock_guard<std::mutex> locker(_send_lock);
        _recive_buffer.clear();
        _send_buffer.clear();
    }
};

//! @endcond

SSLClient::SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port)
    : _id(CppCommon::UUID::Generate()),
      _pimpl(std::make_shared<Impl>(_id, service, context, address, port))
{
}

SSLClient::SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint)
    : _id(CppCommon::UUID::Generate()),
      _pimpl(std::make_shared<Impl>(_id, service, context, endpoint))
{
}

SSLClient::SSLClient(SSLClient&& client)
    : _id(std::move(client._id)),
      _pimpl(std::move(client._pimpl))
{
}

SSLClient::~SSLClient()
{
}

SSLClient& SSLClient::operator=(SSLClient&& client)
{
    _id = std::move(client._id);
    _pimpl = std::move(client._pimpl);
    return *this;
}

std::shared_ptr<Service>& SSLClient::service() noexcept
{
    return _pimpl->service();
}

std::shared_ptr<asio::ssl::context>& SSLClient::context() noexcept
{
    return _pimpl->context();
}

asio::ip::tcp::endpoint& SSLClient::endpoint() noexcept
{
    return _pimpl->endpoint();
}

asio::ssl::stream<asio::ip::tcp::socket>& SSLClient::stream() noexcept
{
    return _pimpl->stream();
}

asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& SSLClient::socket() noexcept
{
    return _pimpl->socket();
}

size_t SSLClient::total_received() const noexcept
{
    return _pimpl->total_received();
}

size_t SSLClient::total_sent() const noexcept
{
    return _pimpl->total_sent();
}

bool SSLClient::IsConnected() const noexcept
{
    return _pimpl->IsConnected();
}

bool SSLClient::IsHandshaked() const noexcept
{
    return _pimpl->IsHandshaked();
}

bool SSLClient::Connect()
{
    auto self(this->shared_from_this());
    return _pimpl->Connect(self);
}

bool SSLClient::Disconnect(bool dispatch)
{
    return _pimpl->Disconnect(dispatch);
}

bool SSLClient::Reconnect()
{
    if (!Disconnect())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return Connect();
}

size_t SSLClient::Send(const void* buffer, size_t size)
{
    return _pimpl->Send(buffer, size);
}

void SSLClient::onReset()
{
    size_t total_received = _pimpl->total_received();
    size_t total_sent = _pimpl->total_sent();
    _pimpl = std::make_shared<Impl>(_pimpl->id(), _pimpl->service(), _pimpl->context(), _pimpl->endpoint());
    _pimpl->total_received() = total_received;
    _pimpl->total_sent() = total_sent;
}

} // namespace Asio
} // namespace CppServer
