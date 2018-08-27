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
          _io_service(_service->GetAsioService()),
          _strand(*_io_service),
          _strand_required(_service->IsStrandRequired()),
          _context(context),
          _endpoint(asio::ip::tcp::endpoint(asio::ip::address::from_string(address), (unsigned short)port)),
          _stream(*_io_service, *_context),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_pending(0),
          _bytes_sending(0),
          _bytes_sent(0),
          _bytes_received(0),
          _reciving(false),
          _sending(false),
          _send_buffer_flush_offset(0),
          _option_keep_alive(false),
          _option_no_delay(false)
    {
        assert((service != nullptr) && "Asio service is invalid!");
        if (service == nullptr)
            throw CppCommon::ArgumentException("Asio service is invalid!");

        assert((context != nullptr) && "SSL context is invalid!");
        if (context == nullptr)
            throw CppCommon::ArgumentException("SSL context is invalid!");
    }

    Impl(const CppCommon::UUID& id, std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint)
        : _id(id),
          _service(service),
          _io_service(_service->GetAsioService()),
          _strand(*_io_service),
          _strand_required(_service->IsStrandRequired()),
          _context(context),
          _endpoint(endpoint),
          _stream(*_io_service, *_context),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_pending(0),
          _bytes_sending(0),
          _bytes_sent(0),
          _bytes_received(0),
          _reciving(false),
          _sending(false),
          _send_buffer_flush_offset(0),
          _option_keep_alive(false),
          _option_no_delay(false)
    {
        assert((service != nullptr) && "Asio service is invalid!");
        if (service == nullptr)
            throw CppCommon::ArgumentException("Asio service is invalid!");

        assert((context != nullptr) && "SSL context is invalid!");
        if (context == nullptr)
            throw CppCommon::ArgumentException("SSL context is invalid!");
    }

    ~Impl() = default;

    const CppCommon::UUID& id() const noexcept { return _id; }

    std::shared_ptr<Service>& service() noexcept { return _service; }
    std::shared_ptr<asio::io_service>& io_service() noexcept { return _io_service; }
    asio::io_service::strand& strand() noexcept { return _strand; }
    std::shared_ptr<asio::ssl::context>& context() noexcept { return _context; }
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    asio::ssl::stream<asio::ip::tcp::socket>& stream() noexcept { return _stream; }
    asio::ssl::stream<asio::ip::tcp::socket>::lowest_layer_type& socket() noexcept { return _stream.lowest_layer(); }

    uint64_t bytes_pending() noexcept { return _bytes_pending + _bytes_sending; }
    uint64_t& bytes_sent() noexcept { return _bytes_sent; }
    uint64_t& bytes_received() noexcept { return _bytes_received; }

    bool option_keep_alive() const noexcept { return _option_keep_alive; }
    bool option_no_delay() const noexcept { return _option_no_delay; }

    size_t option_receive_buffer_size() const
    {
        asio::socket_base::receive_buffer_size option;
        _stream.lowest_layer().get_option(option);
        return option.value();
    }

    size_t option_send_buffer_size() const
    {
        asio::socket_base::send_buffer_size option;
        _stream.lowest_layer().get_option(option);
        return option.value();
    }

    bool IsConnected() const noexcept { return _connected; }
    bool IsHandshaked() const noexcept { return _handshaked; }

    bool Connect(std::shared_ptr<SSLClient> client)
    {
        _client = client;

        if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
            return false;

        // Post the connect handler
        auto self(this->shared_from_this());
        auto connect_handler = make_alloc_handler(_connect_storage, [this, self]()
        {
            if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
                return;

            // Async connect with the connect handler
            _connecting = true;
            auto async_connect_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec1)
            {
                _connecting = false;

                if (IsConnected() || IsHandshaked() || _connecting || _handshaking)
                    return;

                if (!ec1)
                {
                    // Apply the option: keep alive
                    if (option_keep_alive())
                        socket().set_option(asio::ip::tcp::socket::keep_alive(true));
                    // Apply the option: no delay
                    if (option_no_delay())
                        socket().set_option(asio::ip::tcp::no_delay(true));

                    // Prepare receive & send buffers
                    _recive_buffer.resize(option_receive_buffer_size());
                    _send_buffer_main.reserve(option_send_buffer_size());
                    _send_buffer_flush.reserve(option_send_buffer_size());

                    // Reset statistic
                    _bytes_pending = 0;
                    _bytes_sending = 0;
                    _bytes_sent = 0;
                    _bytes_received = 0;

                    // Update the connected flag
                    _connected = true;

                    // Call the client connected handler
                    onConnected();

                    // Async SSL handshake with the handshake handler
                    _handshaking = true;
                    auto async_handshake_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec2)
                    {
                        _handshaking = false;

                        if (IsHandshaked() || _handshaking)
                            return;

                        if (!ec2)
                        {
                            // Update the handshaked flag
                            _handshaked = true;

                            // Call the client handshaked handler
                            onHandshaked();

                            // Call the empty send buffer handler
                            if (_send_buffer_main.empty())
                                onEmpty();

                            // Try to receive something from the server
                            TryReceive();
                        }
                        else
                        {
                            // Disconnect on in case of the bad handshake
                            SendError(ec2);
                            Disconnect(true);
                        }
                    });
                    if (_strand_required)
                        _stream.async_handshake(asio::ssl::stream_base::client, bind_executor(_strand, async_handshake_handler));
                    else
                        _stream.async_handshake(asio::ssl::stream_base::client, async_handshake_handler);
                }
                else
                {
                    // Call the client disconnected handler
                    SendError(ec1);
                    onDisconnected();
                }
            });
            if (_strand_required)
                socket().async_connect(_endpoint, bind_executor(_strand, async_connect_handler));
            else
                socket().async_connect(_endpoint, async_connect_handler);
        });
        if (_strand_required)
            _strand.post(connect_handler);
        else
            _io_service->post(connect_handler);

        return true;
    }

    bool Disconnect(bool dispatch)
    {
        if (!IsConnected() || _connecting || _handshaking)
            return false;

        // Dispatch or post the disconnect handler
        auto self(this->shared_from_this());
        auto disconnect_handler = make_alloc_handler(_connect_storage, [this, self]()
        {
            if (!IsConnected() || _connecting || _handshaking)
                return;

            // Close the client socket
            socket().close();

            // Call the client reset handler
            onReset();

            // Update the handshaked flag
            _handshaked = false;

            // Update the connected flag
            _connected = false;

            // Clear send/receive buffers
            ClearBuffers();

            // Call the client disconnected handler
            onDisconnected();
        });
        if (_strand_required)
        {
            if (dispatch)
                _strand.dispatch(disconnect_handler);
            else
                _strand.post(disconnect_handler);
        }
        else
        {
            if (dispatch)
                _io_service->dispatch(disconnect_handler);
            else
                _io_service->post(disconnect_handler);
        }

        return true;
    }

    bool Send(const void* buffer, size_t size)
    {
        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return false;

        if (!IsHandshaked())
            return false;

        if (size == 0)
            return true;

        {
            std::lock_guard<std::mutex> locker(_send_lock);

            // Detect multiple send handlers
            bool send_required = _send_buffer_main.empty() || _send_buffer_flush.empty();

            // Fill the main send buffer
            const uint8_t* bytes = (const uint8_t*)buffer;
            _send_buffer_main.insert(_send_buffer_main.end(), bytes, bytes + size);

            // Update statistic
            _bytes_pending = _send_buffer_main.size();

            // Avoid multiple send hanlders
            if (!send_required)
                return true;
        }

        // Dispatch the send handler
        auto self(this->shared_from_this());
        auto send_handler = [this, self]()
        {
            // Try to send the main buffer
            TrySend();
        };
        if (_strand_required)
            _strand.dispatch(send_handler);
        else
            _io_service->dispatch(send_handler);

        return true;
    }

    void SetupKeepAlive(bool enable) noexcept { _option_keep_alive = enable; }
    void SetupNoDelay(bool enable) noexcept { _option_no_delay = enable; }

    void SetupReceiveBufferSize(size_t size)
    {
        asio::socket_base::receive_buffer_size option((int)size);
        _stream.lowest_layer().set_option(option);
    }

    void SetupSendBufferSize(size_t size)
    {
        asio::socket_base::send_buffer_size option((int)size);
        _stream.lowest_layer().set_option(option);
    }

protected:
    void onConnected() { _client->onConnected(); }
    void onHandshaked() { _client->onHandshaked(); }
    void onDisconnected() { _client->onDisconnected(); }
    void onReset() { _client->onReset(); }
    void onReceived(const void* buffer, size_t size) { _client->onReceived(buffer, size); }
    void onSent(size_t sent, size_t pending) { _client->onSent(sent, pending); }
    void onEmpty() { _client->onEmpty(); }
    void onError(int error, const std::string& category, const std::string& message) { _client->onError(error, category, message); }

private:
    // Client Id
    CppCommon::UUID _id;
    // SSL client
    std::shared_ptr<SSLClient> _client;
    // Asio service
    std::shared_ptr<Service> _service;
    // Asio IO service
    std::shared_ptr<asio::io_service> _io_service;
    // Asio service strand for serialised handler execution
    asio::io_service::strand _strand;
    bool _strand_required;
    // Server SSL context, endpoint & client stream
    std::shared_ptr<asio::ssl::context> _context;
    asio::ip::tcp::endpoint _endpoint;
    asio::ssl::stream<asio::ip::tcp::socket> _stream;
    std::atomic<bool> _connecting;
    std::atomic<bool> _connected;
    std::atomic<bool> _handshaking;
    std::atomic<bool> _handshaked;
    HandlerStorage _connect_storage;
    // Client statistic
    uint64_t _bytes_pending;
    uint64_t _bytes_sending;
    uint64_t _bytes_sent;
    uint64_t _bytes_received;
    // Receive buffer & cache
    bool _reciving;
    std::vector<uint8_t> _recive_buffer;
    HandlerStorage _recive_storage;
    // Send buffer & cache
    bool _sending;
    std::mutex _send_lock;
    std::vector<uint8_t> _send_buffer_main;
    std::vector<uint8_t> _send_buffer_flush;
    size_t _send_buffer_flush_offset;
    HandlerStorage _send_storage;
    // Options
    bool _option_keep_alive;
    bool _option_no_delay;

    void TryReceive()
    {
        if (_reciving)
            return;

        if (!IsHandshaked())
            return;

        // Async receive with the receive handler
        _reciving = true;
        auto self(this->shared_from_this());
        auto async_receive_handler = make_alloc_handler(_recive_storage, [this, self](std::error_code ec, size_t size)
        {
            _reciving = false;

            if (!IsHandshaked())
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
        if (_strand_required)
            _stream.async_read_some(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), bind_executor(_strand, async_receive_handler));
        else
            _stream.async_read_some(asio::buffer(_recive_buffer.data(), _recive_buffer.size()), async_receive_handler);
    }

    void TrySend()
    {
        if (_sending)
            return;

        if (!IsHandshaked())
            return;

        // Swap send buffers
        if (_send_buffer_flush.empty())
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            // Swap flush and main buffers
            _send_buffer_flush.swap(_send_buffer_main);
            _send_buffer_flush_offset = 0;

            // Update statistic
            _bytes_pending = 0;
            _bytes_sending += _send_buffer_flush.size();
        }
        else
            return;

        // Check if the flush buffer is empty
        if (_send_buffer_flush.empty())
        {
            // Call the empty send buffer handler
            onEmpty();
            return;
        }

        // Async write with the write handler
        _sending = true;
        auto self(this->shared_from_this());
        auto async_write_handler = make_alloc_handler(_send_storage, [this, self](std::error_code ec, size_t size)
        {
            _sending = false;

            if (!IsConnected())
                return;

            // Send some data to the client
            if (size > 0)
            {
                // Update statistic
                _bytes_sending -= size;
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
                onSent(size, bytes_pending());
            }

            // Try to send again if the session is valid
            if (!ec)
                TrySend();
            else
            {
                SendError(ec);
                Disconnect(true);
            }
        });
        if (_strand_required)
            asio::async_write(_stream, asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), bind_executor(_strand, async_write_handler));
        else
            asio::async_write(_stream, asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), async_write_handler);
    }

    void ClearBuffers()
    {
        // Clear send buffers
        {
            std::lock_guard<std::mutex> locker(_send_lock);

            _send_buffer_main.clear();
            _send_buffer_flush.clear();
            _send_buffer_flush_offset = 0;

            // Update statistic
            _bytes_pending = 0;
            _bytes_sending = 0;
        }
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

//! @endcond

SSLClient::SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const std::string& address, int port)
    : _pimpl(std::make_shared<Impl>(CppCommon::UUID::Random(), service, context, address, port))
{
}

SSLClient::SSLClient(std::shared_ptr<Service> service, std::shared_ptr<asio::ssl::context> context, const asio::ip::tcp::endpoint& endpoint)
    : _pimpl(std::make_shared<Impl>(CppCommon::UUID::Random(), service, context, endpoint))
{
}

SSLClient::SSLClient(SSLClient&& client) noexcept
    : _pimpl(std::move(client._pimpl))
{
}

SSLClient::~SSLClient()
{
}

SSLClient& SSLClient::operator=(SSLClient&& client) noexcept
{
    _pimpl = std::move(client._pimpl);
    return *this;
}

const CppCommon::UUID& SSLClient::id() const noexcept
{
    return _pimpl->id();
}

std::shared_ptr<Service>& SSLClient::service() noexcept
{
    return _pimpl->service();
}

std::shared_ptr<asio::io_service>& SSLClient::io_service() noexcept
{
    return _pimpl->io_service();
}

asio::io_service::strand& SSLClient::strand() noexcept
{
    return _pimpl->strand();
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

uint64_t SSLClient::bytes_pending() const noexcept
{
    return _pimpl->bytes_pending();
}

uint64_t SSLClient::bytes_sent() const noexcept
{
    return _pimpl->bytes_sent();
}

uint64_t SSLClient::bytes_received() const noexcept
{
    return _pimpl->bytes_received();
}

bool SSLClient::option_keep_alive() const noexcept
{
    return _pimpl->option_keep_alive();
}

bool SSLClient::option_no_delay() const noexcept
{
    return _pimpl->option_no_delay();
}

size_t SSLClient::option_receive_buffer_size() const
{
    return _pimpl->option_receive_buffer_size();
}

size_t SSLClient::option_send_buffer_size() const
{
    return _pimpl->option_send_buffer_size();
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

bool SSLClient::Send(const void* buffer, size_t size)
{
    return _pimpl->Send(buffer, size);
}

void SSLClient::SetupKeepAlive(bool enable) noexcept
{
    return _pimpl->SetupKeepAlive(enable);
}

void SSLClient::SetupNoDelay(bool enable) noexcept
{
    return _pimpl->SetupNoDelay(enable);
}

void SSLClient::SetupReceiveBufferSize(size_t size)
{
    return _pimpl->SetupReceiveBufferSize(size);
}

void SSLClient::SetupSendBufferSize(size_t size)
{
    return _pimpl->SetupSendBufferSize(size);
}

void SSLClient::onReset()
{
    size_t bytes_sent = _pimpl->bytes_sent();
    size_t bytes_received = _pimpl->bytes_received();
    bool option_keep_alive = _pimpl->option_keep_alive();
    bool option_no_delay = _pimpl->option_no_delay();
    _pimpl = std::make_shared<Impl>(_pimpl->id(), _pimpl->service(), _pimpl->context(), _pimpl->endpoint());
    _pimpl->bytes_sent() = bytes_sent;
    _pimpl->bytes_received() = bytes_received;
    _pimpl->SetupKeepAlive(option_keep_alive);
    _pimpl->SetupNoDelay(option_no_delay);
}

} // namespace Asio
} // namespace CppServer
