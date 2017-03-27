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
          _stream(*_service->service(), *_context),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_sent(0),
          _bytes_received(0),
          _reciving(false),
          _sending(false)
    {
        assert((service != nullptr) && "ASIO service is invalid!");
        if (service == nullptr)
            throw CppCommon::ArgumentException("ASIO service is invalid!");

        assert((context != nullptr) && "SSL context is invalid!");
        if (context == nullptr)
            throw CppCommon::ArgumentException("SSL context is invalid!");
    }

    Impl(const CppCommon::UUID& id, std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint)
        : _id(id),
          _service(service),
          _context(context),
          _endpoint(endpoint),
          _stream(*_service->service(), *_context),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_sent(0),
          _bytes_received(0),
          _reciving(false),
          _sending(false)
    {
        assert((service != nullptr) && "ASIO service is invalid!");
        if (service == nullptr)
            throw CppCommon::ArgumentException("ASIO service is invalid!");

        assert((context != nullptr) && "SSL context is invalid!");
        if (context == nullptr)
            throw CppCommon::ArgumentException("SSL context is invalid!");
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

    uint64_t& bytes_sent() noexcept { return _bytes_sent; }
    uint64_t& bytes_received() noexcept { return _bytes_received; }

    bool IsConnected() const noexcept { return _connected; }
    bool IsHandshaked() const noexcept { return _handshaked; }

    bool Connect(std::shared_ptr<SSLClient>& client)
    {
        _client = client;

        if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
            return false;

        // Post the connect routine
        auto self(this->shared_from_this());
        _service->service()->post([this, self]()
        {
            if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
                return;

            // Connect the client socket
            _connecting = true;
            socket().async_connect(_endpoint, [this, self](std::error_code ec)
            {
                _connecting = false;

                if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
                    return;

                if (!ec)
                {
                    // Reset statistic
                    _bytes_sent = 0;
                    _bytes_received = 0;

                    // Update the connected flag
                    _connected = true;

                    // Call the client connected handler
                    onConnected();

                    // Perform SSL handshake
                    _handshaking = true;
                    _stream.async_handshake(asio::ssl::stream_base::client, [this, self](std::error_code ec)
                    {
                        _handshaking = false;

                        if (IsHandshaked() || _handshaking)
                            return;

                        if (!ec)
                        {
                            // Update the handshaked flag
                            _handshaked = true;

                            // Call the client handshaked handler
                            onHandshaked();

                            // Call the empty send buffer handler
                            onEmpty();

                            // Try to receive something from the server
                            TryReceive();
                        }
                        else
                        {
                            // Disconnect on in case of the bad handshake
                            SendError(ec);
                            Disconnect(true);
                        }
                    });
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

    bool Disconnect(bool dispatch)
    {
        if (!IsConnected() || _connecting || _handshaking)
            return false;

        auto self(this->shared_from_this());
        auto disconnect = [this, self]()
        {
            if (!IsConnected() || _connecting || _handshaking)
                return;

            // Close the client socket
            socket().close();

            // Clear receive/send buffers
            ClearBuffers();

            // Call the client reset handler
            onReset();

            // Update the handshaked flag
            _handshaked = false;

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

    size_t Send(const void* buffer, size_t size)
    {
        assert((buffer != nullptr) && "Pointer to the buffer should not be equal to 'nullptr'!");
        assert((size > 0) && "Buffer size should be greater than zero!");
        if ((buffer == nullptr) || (size == 0))
            return 0;

        if (!IsHandshaked())
            return 0;

        {
            std::lock_guard<std::mutex> locker(_send_lock);

            // Fill the send buffer
            const uint8_t* bytes = (const uint8_t*)buffer;
            _send_cache.insert(_send_cache.end(), bytes, bytes + size);
        }

        // Dispatch the send routine
        auto self(this->shared_from_this());
        _service->Dispatch([this, self]()
        {
            // Try to send the buffer
            TrySend();
        });

        return _send_cache.size();
    }

protected:
    void onConnected() { _client->onConnected(); }
    void onHandshaked() { _client->onHandshaked(); }
    void onDisconnected() { _client->onDisconnected(); }
    void onReset() { _client->onReset(); }
    size_t onReceived(const void* buffer, size_t size) { return _client->onReceived(buffer, size); }
    void onSent(size_t sent, size_t pending) { _client->onSent(sent, pending); }
    void onEmpty() { _client->onEmpty(); }
    void onError(int error, const std::string& category, const std::string& message) { _client->onError(error, category, message); }

private:
    static const size_t CHUNK = 8192;

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
    std::atomic<bool> _connecting;
    std::atomic<bool> _connected;
    std::atomic<bool> _handshaking;
    std::atomic<bool> _handshaked;
    // Client statistic
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive buffer & cache
    bool _reciving;
    uint8_t _recive_buffer[CHUNK];
    std::vector<uint8_t> _recive_cache;
    // Send buffer & cache
    bool _sending;
    std::mutex _send_lock;
    uint8_t _send_buffer[CHUNK];
    std::vector<uint8_t> _send_cache;

    void TryReceive()
    {
        if (_reciving)
            return;

        if (!IsHandshaked())
            return;

        _reciving = true;
        auto self(this->shared_from_this());
        _stream.async_read_some(asio::buffer(_recive_buffer), [this, self](std::error_code ec, std::size_t size)
        {
            _reciving = false;

            if (!IsHandshaked())
                return;

            // Received some data from the client
            if (size > 0)
            {
                // Update statistic
                _bytes_received += size;

                // Fill receive buffer
                _recive_cache.insert(_recive_cache.end(), _recive_buffer, _recive_buffer + size);

                // Call the buffer received handler
                size_t handled = onReceived(_recive_cache.data(), _recive_cache.size());

                // Erase the handled buffer
                _recive_cache.erase(_recive_cache.begin(), _recive_cache.begin() + handled);
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

    void TrySend()
    {
        if (_sending)
            return;

        if (!IsHandshaked())
            return;

        size_t size;
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            // Fill the send buffer
            size = std::min(_send_cache.size(), CHUNK);
            std::memcpy(_send_buffer, _send_cache.data(), size);
        }

        _sending = true;
        auto self(this->shared_from_this());
        asio::async_write(_stream, asio::buffer(_send_buffer, size), [this, self](std::error_code ec, std::size_t size)
        {
            _sending = false;

            if (!IsHandshaked())
                return;

            bool resume = true;

            // Send some data to the client
            if (size > 0)
            {
                // Update statistic
                _bytes_sent += size;

                // Call the buffer sent handler
                onSent(size, _send_cache.size());

                {
                    std::lock_guard<std::mutex> locker(_send_lock);

                    // Erase the sent buffer
                    _send_cache.erase(_send_cache.begin(), _send_cache.begin() + size);

                    // Stop sending if the send buffer is empty
                    if (_send_cache.empty())
                        resume = false;
                }
            }

            // Try to send again if the session is valid
            if (!ec)
            {
                if (resume)
                    TrySend();
                else
                    onEmpty();
            }
            else
            {
                SendError(ec);
                Disconnect(true);
            }
        });
    }

    void ClearBuffers()
    {
        std::lock_guard<std::mutex> locker(_send_lock);

        _recive_cache.clear();
        _send_cache.clear();
    }

    void SendError(std::error_code ec)
    {
        // Skip Asio disconnect errors
        if ((ec == asio::error::connection_aborted) ||
            (ec == asio::error::connection_refused) ||
            (ec == asio::error::connection_reset) ||
            (ec == asio::error::eof) ||
            (ec == asio::error::operation_aborted))
            return;

        // Skip OpenSSL annoying errors
        if (ec == asio::ssl::error::stream_truncated)
            return;
        if (ec.category() == asio::error::get_ssl_category())
        {
            if ((ERR_GET_REASON(ec.value()) == SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC) ||
                (ERR_GET_REASON(ec.value()) == SSL_R_PROTOCOL_IS_SHUTDOWN) ||
                (ERR_GET_REASON(ec.value()) == SSL_R_WRONG_VERSION_NUMBER))
                return;
        }

        onError(ec.value(), ec.category().name(), ec.message());
    }
};

const size_t SSLClient::Impl::CHUNK;

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

uint64_t SSLClient::bytes_sent() const noexcept
{
    return _pimpl->bytes_sent();
}

uint64_t SSLClient::bytes_received() const noexcept
{
    return _pimpl->bytes_received();
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
    size_t bytes_sent = _pimpl->bytes_sent();
    size_t bytes_received = _pimpl->bytes_received();
    _pimpl = std::make_shared<Impl>(_pimpl->id(), _pimpl->service(), _pimpl->context(), _pimpl->endpoint());
    _pimpl->bytes_sent() = bytes_sent;
    _pimpl->bytes_received() = bytes_received;
}

} // namespace Asio
} // namespace CppServer
