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
    Impl(const CppCommon::UUID& id, const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const std::string& address, int port)
        : _id(id),
          _service(service),
          _io_service(_service->GetAsioService()),
          _strand(*_io_service),
          _strand_required(_service->IsStrandRequired()),
          _address(address),
          _port(port),
          _context(context),
          _stream(*_io_service, *_context),
          _resolving(false),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_pending(0),
          _bytes_sending(0),
          _bytes_sent(0),
          _bytes_received(0),
          _receiving(false),
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

    Impl(const CppCommon::UUID& id, const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const std::string& address, const std::string& scheme)
        : _id(id),
          _service(service),
          _io_service(_service->GetAsioService()),
          _strand(*_io_service),
          _strand_required(_service->IsStrandRequired()),
          _address(address),
          _scheme(scheme),
          _port(0),
          _context(context),
          _stream(*_io_service, *_context),
          _resolving(false),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_pending(0),
          _bytes_sending(0),
          _bytes_sent(0),
          _bytes_received(0),
          _receiving(false),
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

    Impl(const CppCommon::UUID& id, const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const asio::ip::tcp::endpoint& endpoint)
        : _id(id),
          _service(service),
          _io_service(_service->GetAsioService()),
          _strand(*_io_service),
          _strand_required(_service->IsStrandRequired()),
          _address(endpoint.address().to_string()),
          _port(endpoint.port()),
          _context(context),
          _endpoint(endpoint),
          _stream(*_io_service, *_context),
          _resolving(false),
          _connecting(false),
          _connected(false),
          _handshaking(false),
          _handshaked(false),
          _bytes_pending(0),
          _bytes_sending(0),
          _bytes_sent(0),
          _bytes_received(0),
          _receiving(false),
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
    std::shared_ptr<SSLContext>& context() noexcept { return _context; }
    asio::ip::tcp::endpoint& endpoint() noexcept { return _endpoint; }
    asio::ssl::stream<asio::ip::tcp::socket>& stream() noexcept { return _stream; }
    asio::ssl::stream<asio::ip::tcp::socket>::next_layer_type& socket() noexcept { return _stream.next_layer(); }

    const std::string& address() const noexcept { return _address; }
    const std::string& scheme() const noexcept { return _scheme; }
    int port() const noexcept { return _port; }

    uint64_t bytes_pending() noexcept { return _bytes_pending + _bytes_sending; }
    uint64_t& bytes_sent() noexcept { return _bytes_sent; }
    uint64_t& bytes_received() noexcept { return _bytes_received; }

    bool option_keep_alive() const noexcept { return _option_keep_alive; }
    bool option_no_delay() const noexcept { return _option_no_delay; }

    size_t option_receive_buffer_size() const
    {
        asio::socket_base::receive_buffer_size option;
        _stream.next_layer().get_option(option);
        return option.value();
    }

    size_t option_send_buffer_size() const
    {
        asio::socket_base::send_buffer_size option;
        _stream.next_layer().get_option(option);
        return option.value();
    }

    bool IsConnected() const noexcept { return _connected; }
    bool IsHandshaked() const noexcept { return _handshaked; }

    bool Connect(const std::shared_ptr<SSLClient>& client)
    {
        // Link the client
        _client = client;

        if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
            return false;

        asio::error_code ec;

        // Create the server endpoint
        _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(_address), (unsigned short)_port);

        // Connect to the server
        socket().connect(_endpoint, ec);

        // Disconnect on error
        if (ec)
        {
            SendError(ec);

            // Call the client disconnected handler
            onDisconnected();

            return false;
        }

        // Apply the option: keep alive
        if (option_keep_alive())
            socket().set_option(asio::ip::tcp::socket::keep_alive(true));
        // Apply the option: no delay
        if (option_no_delay())
            socket().set_option(asio::ip::tcp::no_delay(true));

        // Prepare receive & send buffers
        _receive_buffer.resize(option_receive_buffer_size());
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

        // SSL handshake
        _stream.handshake(asio::ssl::stream_base::client, ec);

        // Disconnect on error
        if (ec)
        {
            // Disconnect in case of the bad handshake
            SendError(ec);
            Disconnect();
            return false;
        }

        // Update the handshaked flag
        _handshaked = true;

        // Call the client handshaked handler
        onHandshaked();

        // Call the empty send buffer handler
        if (_send_buffer_main.empty())
            onEmpty();

        return true;
    }

    bool Connect(const std::shared_ptr<SSLClient>& client, const std::shared_ptr<TCPResolver>& resolver)
    {
        // Link the client
        _client = client;

        if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
            return false;

        asio::error_code ec;

        // Resolve the server endpoint
        asio::ip::tcp::resolver::query query(_address, (_scheme.empty() ? std::to_string(_port) : _scheme));
        auto endpoints = resolver->resolver().resolve(query, ec);

        // Disconnect on error
        if (ec)
        {
            SendError(ec);

            // Call the client disconnected handler
            onDisconnected();

            return false;
        }

        //  Connect to the server
        _endpoint = asio::connect(socket(), endpoints, ec);

        // Disconnect on error
        if (ec)
        {
            SendError(ec);

            // Call the client disconnected handler
            onDisconnected();

            return false;
        }

        // Apply the option: keep alive
        if (option_keep_alive())
            socket().set_option(asio::ip::tcp::socket::keep_alive(true));
        // Apply the option: no delay
        if (option_no_delay())
            socket().set_option(asio::ip::tcp::no_delay(true));

        // Prepare receive & send buffers
        _receive_buffer.resize(option_receive_buffer_size());
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

        // SSL handshake
        _stream.handshake(asio::ssl::stream_base::client, ec);

        // Disconnect on error
        if (ec)
        {
            // Disconnect in case of the bad handshake
            SendError(ec);
            Disconnect();
            return false;
        }

        // Update the handshaked flag
        _handshaked = true;

        // Call the client handshaked handler
        onHandshaked();

        // Call the empty send buffer handler
        if (_send_buffer_main.empty())
            onEmpty();

        return true;
    }

    bool Disconnect()
    {
        if (!IsConnected() || _resolving || _connecting || _handshaking)
        {
            // Unlink the client
            _client = nullptr;
            return false;
        }

        auto self(this->shared_from_this());

        // Close the client socket
        socket().close();

        // Call the client reset handler
        onReset();

        // Update the handshaked flag
        _handshaking = false;
        _handshaked = false;

        // Update the connected flag
        _resolving = false;
        _connecting = false;
        _connected = false;

        // Update sending/receiving flags
        _receiving = false;
        _sending = false;

        // Clear send/receive buffers
        ClearBuffers();

        // Call the client disconnected handler
        onDisconnected();

        // Unlink the client
        _client = nullptr;

        return true;
    }

    bool ConnectAsync(const std::shared_ptr<SSLClient>& client)
    {
        // Link the client
        _client = client;

        if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
            return false;

        // Post the connect handler
        auto self(this->shared_from_this());
        auto connect_handler = make_alloc_handler(_connect_storage, [this, self]()
        {
            if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
                return;

            // Async connect with the connect handler
            _connecting = true;
            auto async_connect_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec1)
            {
                _connecting = false;

                if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
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
                    _receive_buffer.resize(option_receive_buffer_size());
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

                        if (IsHandshaked())
                            return;

                        if (!ec2)
                        {
                            // Update the handshaked flag
                            _handshaked = true;

                            // Try to receive something from the server
                            TryReceive();

                            // Call the client handshaked handler
                            onHandshaked();

                            // Call the empty send buffer handler
                            if (_send_buffer_main.empty())
                                onEmpty();
                        }
                        else
                        {
                            // Disconnect in case of the bad handshake
                            SendError(ec2);
                            DisconnectAsync(true);
                        }
                    });
                    if (_strand_required)
                        _stream.async_handshake(asio::ssl::stream_base::client, bind_executor(_strand, async_handshake_handler));
                    else
                        _stream.async_handshake(asio::ssl::stream_base::client, async_handshake_handler);
                }
                else
                {
                    SendError(ec1);

                    // Call the client disconnected handler
                    onDisconnected();
                }
            });

            // Create the server endpoint
            _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(_address), (unsigned short)_port);

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

    bool ConnectAsync(const std::shared_ptr<SSLClient>& client, const std::shared_ptr<TCPResolver>& resolver)
    {
        // Link the client
        _client = client;

        if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
            return false;

        // Post the connect handler
        auto self(this->shared_from_this());
        auto connect_handler = make_alloc_handler(_connect_storage, [this, self, resolver]()
        {
            if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
                return;

            // Async resolve with the resolve handler
            _resolving = true;
            auto async_resolve_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec1, asio::ip::tcp::resolver::results_type endpoints)
            {
                _resolving = false;

                if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
                    return;

                if (!ec1)
                {
                    // Async connect with the connect handler
                    _connecting = true;
                    auto async_connect_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec2, const asio::ip::tcp::endpoint& endpoint)
                    {
                        _connecting = false;

                        if (IsConnected() || IsHandshaked() || _resolving || _connecting || _handshaking)
                            return;

                        if (!ec2)
                        {
                            //  Connect to the server
                            _endpoint = endpoint;

                            // Apply the option: keep alive
                            if (option_keep_alive())
                                socket().set_option(asio::ip::tcp::socket::keep_alive(true));
                            // Apply the option: no delay
                            if (option_no_delay())
                                socket().set_option(asio::ip::tcp::no_delay(true));

                            // Prepare receive & send buffers
                            _receive_buffer.resize(option_receive_buffer_size());
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
                            auto async_handshake_handler = make_alloc_handler(_connect_storage, [this, self](std::error_code ec3)
                            {
                                _handshaking = false;

                                if (IsHandshaked())
                                    return;

                                if (!ec3)
                                {
                                    // Update the handshaked flag
                                    _handshaked = true;

                                    // Try to receive something from the server
                                    TryReceive();

                                    // Call the client handshaked handler
                                    onHandshaked();

                                    // Call the empty send buffer handler
                                    if (_send_buffer_main.empty())
                                        onEmpty();
                                }
                                else
                                {
                                    // Disconnect in case of the bad handshake
                                    SendError(ec3);
                                    DisconnectAsync(true);
                                }
                            });
                            if (_strand_required)
                                _stream.async_handshake(asio::ssl::stream_base::client, bind_executor(_strand, async_handshake_handler));
                            else
                                _stream.async_handshake(asio::ssl::stream_base::client, async_handshake_handler);
                        }
                        else
                        {
                            SendError(ec2);

                            // Call the client disconnected handler
                            onDisconnected();
                        }
                    });
                    if (_strand_required)
                        asio::async_connect(socket(), endpoints, bind_executor(_strand, async_connect_handler));
                    else
                        asio::async_connect(socket(), endpoints, async_connect_handler);
                }
                else
                {
                    SendError(ec1);

                    // Call the client disconnected handler
                    onDisconnected();
                }
            });

            // Resolve the server endpoint
            asio::ip::tcp::resolver::query query(_address, (_scheme.empty() ? std::to_string(_port) : _scheme));
            if (_strand_required)
                resolver->resolver().async_resolve(query, bind_executor(_strand, async_resolve_handler));
            else
                resolver->resolver().async_resolve(query, async_resolve_handler);
        });
        if (_strand_required)
            _strand.post(connect_handler);
        else
            _io_service->post(connect_handler);

        return true;
    }

    bool DisconnectAsync(bool dispatch)
    {
        if (!IsConnected() || _resolving || _connecting || _handshaking)
        {
            // Unlink the client
            _client = nullptr;
            return false;
        }

        // Dispatch or post the disconnect handler
        auto self(this->shared_from_this());
        auto disconnect_handler = make_alloc_handler(_connect_storage, [this, self]() { Disconnect(); });
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

    size_t Send(const void* buffer, size_t size)
    {
        if (!IsHandshaked())
            return 0;

        if (size == 0)
            return 0;

        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return 0;

        asio::error_code ec;

        // Send data to the server
        size_t sent = asio::write(_stream, asio::buffer(buffer, size), ec);
        if (sent > 0)
        {
            // Update statistic
            _bytes_sent += sent;

            // Call the buffer sent handler
            onSent(sent, bytes_pending());
        }

        // Disconnect on error
        if (ec)
        {
            SendError(ec);
            Disconnect();
        }

        return sent;
    }

    size_t Send(const void* buffer, size_t size, const CppCommon::Timespan& timeout)
    {
        if (!IsHandshaked())
            return 0;

        if (size == 0)
            return 0;

        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return 0;

        int done = 0;
        std::mutex mtx;
        std::condition_variable cv;
        asio::error_code error;
        asio::system_timer timer(_stream.get_executor());

        // Prepare done handler
        auto async_done_handler = [&](asio::error_code ec)
        {
            std::unique_lock<std::mutex> lck(mtx);
            if (done++ == 0)
            {
                error = ec;
                _stream.next_layer().cancel();
                timer.cancel();
            }
            cv.notify_one();
        };

        // Async wait for timeout
        timer.expires_from_now(timeout.chrono());
        timer.async_wait([&](const asio::error_code& ec) { async_done_handler(ec ? ec : asio::error::timed_out); });

        // Async write some data to the server
        size_t sent = 0;
        _stream.async_write_some(asio::buffer(buffer, size), [&](std::error_code ec, size_t write) { async_done_handler(ec); sent = write; });

        // Wait for complete or timeout
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [&]() { return done == 2; });

        // Send data to the server
        if (sent > 0)
        {
            // Update statistic
            _bytes_sent += sent;

            // Call the buffer sent handler
            onSent(sent, bytes_pending());
        }

        // Disconnect on error
        if (error && (error != asio::error::timed_out))
        {
            SendError(error);
            Disconnect();
        }

        return sent;
    }

    bool SendAsync(const void* buffer, size_t size)
    {
        if (!IsHandshaked())
            return false;

        if (size == 0)
            return true;

        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return false;

        {
            std::scoped_lock locker(_send_lock);

            // Detect multiple send handlers
            bool send_required = _send_buffer_main.empty() || _send_buffer_flush.empty();

            // Fill the main send buffer
            const uint8_t* bytes = (const uint8_t*)buffer;
            _send_buffer_main.insert(_send_buffer_main.end(), bytes, bytes + size);

            // Update statistic
            _bytes_pending = _send_buffer_main.size();

            // Avoid multiple send handlers
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

    size_t Receive(void* buffer, size_t size)
    {
        if (!IsHandshaked())
            return 0;

        if (size == 0)
            return 0;

        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return 0;

        asio::error_code ec;

        // Receive data from the server
        size_t received = _stream.read_some(asio::buffer(buffer, size), ec);
        if (received > 0)
        {
            // Update statistic
            _bytes_received += received;

            // Call the buffer received handler
            onReceived(buffer, received);
        }

        // Disconnect on error
        if (ec)
        {
            SendError(ec);
            Disconnect();
        }

        return received;
    }

    std::string Receive(size_t size)
    {
        std::string text(size, 0);
        text.resize(Receive(text.data(), text.size()));
        return text;
    }

    size_t Receive(void* buffer, size_t size, const CppCommon::Timespan& timeout)
    {
        if (!IsHandshaked())
            return 0;

        if (size == 0)
            return 0;

        assert((buffer != nullptr) && "Pointer to the buffer should not be null!");
        if (buffer == nullptr)
            return 0;

        int done = 0;
        std::mutex mtx;
        std::condition_variable cv;
        asio::error_code error;
        asio::system_timer timer(_stream.get_executor());

        // Prepare done handler
        auto async_done_handler = [&](asio::error_code ec)
        {
            std::unique_lock<std::mutex> lck(mtx);
            if (done++ == 0)
            {
                error = ec;
                _stream.next_layer().cancel();
                timer.cancel();
            }
            cv.notify_one();
        };

        // Async wait for timeout
        timer.expires_from_now(timeout.chrono());
        timer.async_wait([&](const asio::error_code& ec) { async_done_handler(ec ? ec : asio::error::timed_out); });

        // Async read some data from the server
        size_t received = 0;
        _stream.async_read_some(asio::buffer(buffer, size), [&](std::error_code ec, size_t read) { async_done_handler(ec); received = read; });

        // Wait for complete or timeout
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, [&]() { return done == 2; });

        // Received some data from the server
        if (received > 0)
        {
            // Update statistic
            _bytes_received += received;

            // Call the buffer received handler
            onReceived(buffer, received);
        }

        // Disconnect on error
        if (error && (error != asio::error::timed_out))
        {
            SendError(error);
            Disconnect();
        }

        return received;
    }

    std::string Receive(size_t size, const CppCommon::Timespan& timeout)
    {
        std::string text(size, 0);
        text.resize(Receive(text.data(), text.size(), timeout));
        return text;
    }

    void ReceiveAsync()
    {
        // Try to receive data from the server
        TryReceive();
    }

    void SetupKeepAlive(bool enable) noexcept { _option_keep_alive = enable; }
    void SetupNoDelay(bool enable) noexcept { _option_no_delay = enable; }

    void SetupReceiveBufferSize(size_t size)
    {
        asio::socket_base::receive_buffer_size option((int)size);
        _stream.next_layer().set_option(option);
    }

    void SetupSendBufferSize(size_t size)
    {
        asio::socket_base::send_buffer_size option((int)size);
        _stream.next_layer().set_option(option);
    }

protected:
    void onConnected() { if (_client) _client->onConnected(); }
    void onHandshaked() { if (_client) _client->onHandshaked(); }
    void onDisconnected() { if (_client) _client->onDisconnected(); }
    void onReset() { if (_client) _client->onReset(); }
    void onReceived(const void* buffer, size_t size) { if (_client) _client->onReceived(buffer, size); }
    void onSent(size_t sent, size_t pending) { if (_client) _client->onSent(sent, pending); }
    void onEmpty() { if (_client) _client->onEmpty(); }
    void onError(int error, const std::string& category, const std::string& message) { if (_client) _client->onError(error, category, message); }

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
    // Server address, scheme & port
    std::string _address;
    std::string _scheme;
    int _port;
    // Server SSL context, endpoint & client stream
    std::shared_ptr<SSLContext> _context;
    asio::ip::tcp::endpoint _endpoint;
    asio::ssl::stream<asio::ip::tcp::socket> _stream;
    std::atomic<bool> _resolving;
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
    // Receive buffer
    bool _receiving;
    std::vector<uint8_t> _receive_buffer;
    HandlerStorage _receive_storage;
    // Send buffer
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
        if (_receiving)
            return;

        if (!IsHandshaked())
            return;

        // Async receive with the receive handler
        _receiving = true;
        auto self(this->shared_from_this());
        auto async_receive_handler = make_alloc_handler(_receive_storage, [this, self](std::error_code ec, size_t size)
        {
            _receiving = false;

            if (!IsHandshaked())
                return;

            // Received some data from the server
            if (size > 0)
            {
                // Update statistic
                _bytes_received += size;

                // Call the buffer received handler
                onReceived(_receive_buffer.data(), size);

                // If the receive buffer is full increase its size
                if (_receive_buffer.size() == size)
                    _receive_buffer.resize(2 * size);
            }

            // Try to receive again if the session is valid
            if (!ec)
                TryReceive();
            else
            {
                SendError(ec);
                DisconnectAsync(true);
            }
        });
        if (_strand_required)
            _stream.async_read_some(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), bind_executor(_strand, async_receive_handler));
        else
            _stream.async_read_some(asio::buffer(_receive_buffer.data(), _receive_buffer.size()), async_receive_handler);
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
            std::scoped_lock locker(_send_lock);

            // Swap flush and main buffers
            _send_buffer_flush.swap(_send_buffer_main);
            _send_buffer_flush_offset = 0;

            // Update statistic
            _bytes_pending = 0;
            _bytes_sending += _send_buffer_flush.size();
        }

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

            if (!IsHandshaked())
                return;

            // Send some data to the server
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
                DisconnectAsync(true);
            }
        });
        if (_strand_required)
            _stream.async_write_some(asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), bind_executor(_strand, async_write_handler));
        else
            _stream.async_write_some(asio::buffer(_send_buffer_flush.data() + _send_buffer_flush_offset, _send_buffer_flush.size() - _send_buffer_flush_offset), async_write_handler);
    }

    void ClearBuffers()
    {
        {
            std::scoped_lock locker(_send_lock);

            // Clear send buffers
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

SSLClient::SSLClient(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const std::string& address, int port)
{
    _pimpl = std::make_shared<Impl>(CppCommon::UUID::Sequential(), service, context, address, port);
}

SSLClient::SSLClient(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const std::string& address, const std::string& scheme)
{
    _pimpl = std::make_shared<Impl>(CppCommon::UUID::Sequential(), service, context, address, scheme);
}

SSLClient::SSLClient(const std::shared_ptr<Service>& service, const std::shared_ptr<SSLContext>& context, const asio::ip::tcp::endpoint& endpoint)
{
    _pimpl = std::make_shared<Impl>(CppCommon::UUID::Sequential(), service, context, endpoint);
}

SSLClient::~SSLClient()
{
}

const CppCommon::UUID& SSLClient::id() const noexcept { return pimpl()->id(); }

std::shared_ptr<Service>& SSLClient::service() noexcept { return pimpl()->service(); }
std::shared_ptr<asio::io_service>& SSLClient::io_service() noexcept { return pimpl()->io_service(); }
asio::io_service::strand& SSLClient::strand() noexcept { return pimpl()->strand(); }
std::shared_ptr<SSLContext>& SSLClient::context() noexcept { return pimpl()->context(); }
asio::ip::tcp::endpoint& SSLClient::endpoint() noexcept { return pimpl()->endpoint(); }
asio::ssl::stream<asio::ip::tcp::socket>& SSLClient::stream() noexcept { return pimpl()->stream(); }
asio::ssl::stream<asio::ip::tcp::socket>::next_layer_type& SSLClient::socket() noexcept { return pimpl()->socket(); }

const std::string& SSLClient::address() const noexcept { return pimpl()->address(); }
const std::string& SSLClient::scheme() const noexcept { return pimpl()->scheme(); }
int SSLClient::port() const noexcept { return pimpl()->port(); }

uint64_t SSLClient::bytes_pending() const noexcept { return pimpl()->bytes_pending(); }
uint64_t SSLClient::bytes_sent() const noexcept { return pimpl()->bytes_sent(); }
uint64_t SSLClient::bytes_received() const noexcept { return pimpl()->bytes_received(); }

bool SSLClient::option_keep_alive() const noexcept { return pimpl()->option_keep_alive(); }
bool SSLClient::option_no_delay() const noexcept { return pimpl()->option_no_delay(); }
size_t SSLClient::option_receive_buffer_size() const { return pimpl()->option_receive_buffer_size(); }
size_t SSLClient::option_send_buffer_size() const { return pimpl()->option_send_buffer_size(); }

bool SSLClient::IsConnected() const noexcept { return pimpl()->IsConnected(); }
bool SSLClient::IsHandshaked() const noexcept { return pimpl()->IsHandshaked(); }

bool SSLClient::Connect()
{
    auto self(this->shared_from_this());
    return _pimpl->Connect(self);
}

bool SSLClient::Connect(const std::shared_ptr<TCPResolver>& resolver)
{
    auto self(this->shared_from_this());
    return _pimpl->Connect(self, resolver);
}

bool SSLClient::Disconnect()
{
    return _pimpl->Disconnect();
}

bool SSLClient::Reconnect()
{
    if (!Disconnect())
        return false;

    return Connect();
}

bool SSLClient::ConnectAsync()
{
    auto self(this->shared_from_this());
    return _pimpl->ConnectAsync(self);
}

bool SSLClient::ConnectAsync(const std::shared_ptr<TCPResolver>& resolver)
{
    auto self(this->shared_from_this());
    return _pimpl->ConnectAsync(self, resolver);
}

bool SSLClient::DisconnectAsync(bool dispatch)
{
    return _pimpl->DisconnectAsync(dispatch);
}

bool SSLClient::ReconnectAsync()
{
    if (!DisconnectAsync())
        return false;

    while (IsConnected())
        CppCommon::Thread::Yield();

    return ConnectAsync();
}

size_t SSLClient::Send(const void* buffer, size_t size) { return pimpl()->Send(buffer, size); }
size_t SSLClient::Send(const void* buffer, size_t size, const CppCommon::Timespan& timeout) { return pimpl()->Send(buffer, size, timeout); }
bool SSLClient::SendAsync(const void* buffer, size_t size) { return pimpl()->SendAsync(buffer, size); }
size_t SSLClient::Receive(void* buffer, size_t size) { return pimpl()->Receive(buffer, size); }
std::string SSLClient::Receive(size_t size) { return pimpl()->Receive(size); }
size_t SSLClient::Receive(void* buffer, size_t size, const CppCommon::Timespan& timeout) { return pimpl()->Receive(buffer, size, timeout); }
std::string SSLClient::Receive(size_t size, const CppCommon::Timespan& timeout) { return pimpl()->Receive(size, timeout); }
void SSLClient::ReceiveAsync() { return pimpl()->ReceiveAsync(); }

void SSLClient::SetupKeepAlive(bool enable) noexcept { return pimpl()->SetupKeepAlive(enable); }
void SSLClient::SetupNoDelay(bool enable) noexcept { return pimpl()->SetupNoDelay(enable); }
void SSLClient::SetupReceiveBufferSize(size_t size) { return pimpl()->SetupReceiveBufferSize(size); }
void SSLClient::SetupSendBufferSize(size_t size) { return pimpl()->SetupSendBufferSize(size); }

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
