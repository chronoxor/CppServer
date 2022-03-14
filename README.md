# CppServer

[![Awesome C++](https://awesome.re/badge.svg)](https://github.com/fffaraz/awesome-cpp)
<br/>
[![Linux (clang)](https://github.com/chronoxor/CppServer/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/chronoxor/CppServer/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-linux-gcc.yml)
<br/>
[![MacOS](https://github.com/chronoxor/CppServer/actions/workflows/build-macos.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-macos.yml)
<br/>
[![Windows (MSYS2)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-msys2.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-msys2.yml)
[![Windows (MinGW)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-mingw.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-mingw.yml)
[![Windows (Visual Studio)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-vs.yml/badge.svg)](https://github.com/chronoxor/CppServer/actions/workflows/build-windows-vs.yml)

Ultra fast and low latency asynchronous socket server & client C++ library with
support TCP, SSL, UDP, HTTP, HTTPS, WebSocket protocols and [10K connections problem](https://en.wikipedia.org/wiki/C10k_problem)
solution.

Has integration with high-level message protocol based on [Fast Binary Encoding](https://github.com/chronoxor/FastBinaryEncoding)

[CppServer API reference](https://chronoxor.github.io/CppServer/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
  * [Examples](#examples)
    * [Example: Asio service](#example-asio-service)
    * [Example: Asio timer](#example-asio-timer)
    * [Example: TCP chat server](#example-tcp-chat-server)
    * [Example: TCP chat client](#example-tcp-chat-client)
    * [Example: SSL chat server](#example-ssl-chat-server)
    * [Example: SSL chat client](#example-ssl-chat-client)
    * [Example: UDP echo server](#example-udp-echo-server)
    * [Example: UDP echo client](#example-udp-echo-client)
    * [Example: UDP multicast server](#example-udp-multicast-server)
    * [Example: UDP multicast client](#example-udp-multicast-client)
    * [Example: Simple protocol](#example-simple-protocol)
    * [Example: Simple protocol server](#example-simple-protocol-server)
    * [Example: Simple protocol client](#example-simple-protocol-client)
    * [Example: HTTP server](#example-http-server)
    * [Example: HTTP client](#example-http-client)
    * [Example: HTTPS server](#example-https-server)
    * [Example: HTTPS client](#example-https-client)
    * [Example: WebSocket chat server](#example-websocket-chat-server)
    * [Example: WebSocket chat client](#example-websocket-chat-client)
    * [Example: WebSocket secure chat server](#example-websocket-secure-chat-server)
    * [Example: WebSocket secure chat client](#example-websocket-secure-chat-client)
  * [Performance](#performance)
    * [Benchmark: Round-Trip](#benchmark-round-trip)
      * [TCP echo server](#tcp-echo-server)
      * [SSL echo server](#ssl-echo-server)
      * [UDP echo server](#udp-echo-server)
      * [Simple protocol server](#simple-protocol-server)
      * [WebSocket echo server](#websocket-echo-server)
      * [WebSocket secure echo server](#websocket-secure-echo-server)
    * [Benchmark: Multicast](#benchmark-multicast)
      * [TCP multicast server](#tcp-multicast-server)
      * [SSL multicast server](#ssl-multicast-server)
      * [UDP multicast server](#udp-multicast-server)
      * [WebSocket multicast server](#websocket-multicast-server)
      * [WebSocket secure multicast server](#websocket-secure-multicast-server)
    * [Benchmark: Web Server](#benchmark-web-server)
      * [HTTP Trace server](#http-trace-server)
      * [HTTPS Trace server](#https-trace-server)
  * [OpenSSL certificates](#openssl-certificates)
    * [Production](#production)
    * [Development](#development)
    * [Certificate Authority](#certificate-authority)
    * [SSL Server certificate](#ssl-server-certificate)
    * [SSL Client certificate](#ssl-client-certificate)
    * [Diffie-Hellman key exchange](#diffie-hellman-key-exchange)

# Features
* Cross platform (Linux, MacOS, Windows)
* [Asynchronous communication](https://think-async.com)
* Supported CPU scalability designs: IO service per thread, thread pool
* Supported transport protocols: [TCP](#example-tcp-chat-server), [SSL](#example-ssl-chat-server),
  [UDP](#example-udp-echo-server), [UDP multicast](#example-udp-multicast-server)
* Supported Web protocols: [HTTP](#example-http-server), [HTTPS](#example-https-server),
  [WebSocket](#example-websocket-chat-server), [WebSocket secure](#example-websocket-secure-chat-server)
* Supported [Swagger OpenAPI](https://swagger.io/specification/) iterative documentation
* Supported message protocol based on [Fast Binary Encoding](https://github.com/chronoxor/FastBinaryEncoding)

# Requirements
* Linux
* MacOS
* Windows
* [cmake](https://www.cmake.org)
* [gcc](https://gcc.gnu.org)
* [git](https://git-scm.com)
* [gil](https://github.com/chronoxor/gil.git)
* [python3](https://www.python.org)

Optional:
* [clang](https://clang.llvm.org)
* [CLion](https://www.jetbrains.com/clion)
* [MSYS2](https://www.msys2.org)
* [MinGW](https://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

### Linux: install required packages
```shell
sudo apt-get install -y binutils-dev uuid-dev libssl-dev
```

### Install [gil (git links) tool](https://github.com/chronoxor/gil)
```shell
pip3 install gil
```

### Setup repository
```shell
git clone https://github.com/chronoxor/CppServer.git
cd CppServer
gil update
```

### Linux
```shell
cd build
./unix.sh
```

### MacOS
```shell
cd build
./unix.sh
```

### Windows (MSYS2)
```shell
cd build
unix.bat
```

### Windows (MinGW)
```shell
cd build
mingw.bat
```

### Windows (Visual Studio)
```shell
cd build
vs.bat
```

# Examples

## Example: Asio service
Asio service is used to host all clients/servers based on [Asio C++ library](https://think-async.com).
It is implemented based on Asio C++ Library and use a separate thread to
perform all asynchronous IO operations and communications.

The common usecase is to instantiate one Asio service, start the service and
attach TCP/UDP/WebSocket servers or/and clients to it. One Asio service can
handle several servers and clients asynchronously at the same time in one I/O
thread. If you want to scale your servers or clients it is possible to create
and use more than one Asio services to handle your servers/clients in balance.

Also it is possible to dispatch or post your custom handler into I/O thread.
Dispatch will execute the handler immediately if the current thread is I/O one.
Otherwise the handler will be enqueued to the I/O queue. In opposite the post
method will always enqueue the handler into the I/O queue.

Here comes an example of using custom Asio service with dispatch/post methods:
```c++
#include "server/asio/service.h"
#include "threads/thread.h"

#include <iostream>

int main(int argc, char** argv)
{
    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Dispatch
    std::cout << "1 - Dispatch from the main thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
    service->Dispatch([service]()
    {
        std::cout << "1.1 - Dispatched in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;

        std::cout << "1.2 - Dispatch from thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        service->Dispatch([service]()
        {
            std::cout << "1.2.1 - Dispatched in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        });

        std::cout << "1.3 - Post from thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        service->Post([service]()
        {
            std::cout << "1.3.1 - Posted in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        });
    });

    // Post
    std::cout << "2 - Post from the main thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
    service->Post([service]()
    {
        std::cout << "2.1 - Posted in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;

        std::cout << "2.2 - Dispatch from thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        service->Dispatch([service]()
        {
            std::cout << "2.2.1 - Dispatched in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        });

        std::cout << "2.3 - Post from thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        service->Post([service]()
        {
            std::cout << "2.3.1 - Posted in thread with Id " << CppCommon::Thread::CurrentThreadId() << std::endl;
        });
    });

    // Wait for a while...
    CppCommon::Thread::Sleep(1000);

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

Output of the above example is the following:
```
Asio service started!
1 - Dispatch from the main thread with Id 16744
2 - Post from the main thread with Id 16744
1.1 - Dispatched in thread with Id 19920
1.2 - Dispatch from thread with Id 19920
1.2.1 - Dispatched in thread with Id 19920
1.3 - Post from thread with Id 19920
2.1 - Posted in thread with Id 19920
2.2 - Dispatch from thread with Id 19920
2.2.1 - Dispatched in thread with Id 19920
2.3 - Post from thread with Id 19920
1.3.1 - Posted in thread with Id 19920
2.3.1 - Posted in thread with Id 19920
Asio service stopped!
```

## Example: Asio timer
Here comes the example of Asio timer. It can be used to wait for some action
in future with providing absolute time or relative time span. Asio timer can
be used in synchronous or asynchronous modes.
```c++
#include "server/asio/timer.h"
#include "threads/thread.h"

#include <iostream>

class AsioTimer : public CppServer::Asio::Timer
{
public:
    using CppServer::Asio::Timer::Timer;

protected:
    void onTimer(bool canceled) override
    {
        std::cout << "Asio timer " << (canceled ? "canceled" : "expired") << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Asio timer caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new Asio timer
    auto timer = std::make_shared<AsioTimer>(service);

    // Setup and synchronously wait for the timer
    timer->Setup(CppCommon::UtcTime() + CppCommon::Timespan::seconds(1));
    timer->WaitSync();

    // Setup and asynchronously wait for the timer
    timer->Setup(CppCommon::Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    CppCommon::Thread::Sleep(2000);

    // Setup and asynchronously wait for the timer
    timer->Setup(CppCommon::Timespan::seconds(1));
    timer->WaitAsync();

    // Wait for a while...
    CppCommon::Thread::Sleep(500);

    // Cancel the timer
    timer->Cancel();

    // Wait for a while...
    CppCommon::Thread::Sleep(500);

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

Output of the above example is the following:
```
Asio service starting...Done!
Timer was expired
Timer was canceled
Asio service stopping...Done!
```

## Example: TCP chat server
Here comes the example of the TCP chat server. It handles multiple TCP client
sessions and multicast received message from any session to all ones. Also it
is possible to send admin message directly from the server.

```c++
#include "server/asio/tcp_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession : public CppServer::Asio::TCPSession
{
public:
    using CppServer::Asio::TCPSession::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from TCP chat! Please send a message or '!' to disconnect the client!");
        SendAsync(message);
    }

    void onDisconnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            DisconnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatServer : public CppServer::Asio::TCPServer
{
public:
    using CppServer::Asio::TCPServer::TCPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(std::shared_ptr<CppServer::Asio::TCPServer> server) override
    {
        return std::make_shared<ChatSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // TCP server port
    int port = 1111;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "TCP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new TCP chat server
    auto server = std::make_shared<ChatServer>(service, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: TCP chat client
Here comes the example of the TCP chat client. It connects to the TCP chat
server and allows to send message to it and receive new messages.

```c++
#include "server/asio/tcp_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::Asio::TCPClient
{
public:
    ChatClient(std::shared_ptr<CppServer::Asio::Service> service, const std::string& address, int port)
        : CppServer::Asio::TCPClient(service, address, port)
    {
        _stop = false;
    }

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onConnected() override
    {
        std::cout << "Chat TCP client connected a new session with Id " << id() << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Chat TCP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop;
};

int main(int argc, char** argv)
{
    // TCP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // TCP server port
    int port = 1111;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "TCP server address: " << address << std::endl;
    std::cout << "TCP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new TCP chat client
    auto client = std::make_shared<ChatClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->DisconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: SSL chat server
Here comes the example of the SSL chat server. It handles multiple SSL client
sessions and multicast received message from any session to all ones. Also it
is possible to send admin message directly from the server.

This example is very similar to the TCP one except the code that prepares SSL
context and handshake handler.

```c++
#include "server/asio/ssl_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession : public CppServer::Asio::SSLSession
{
public:
    using CppServer::Asio::SSLSession::SSLSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " connected!" << std::endl;
    }

    void onHandshaked() override
    {
        std::cout << "Chat SSL session with Id " << id() << " handshaked!" << std::endl;

        // Send invite message
        std::string message("Hello from SSL chat! Please send a message or '!' to disconnect the client!");
        SendAsync(message.data(), message.size());
    }

    void onDisconnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            DisconnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatServer : public CppServer::Asio::SSLServer
{
public:
    using CppServer::Asio::SSLServer::SSLServer;

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(std::shared_ptr<CppServer::Asio::SSLServer> server) override
    {
        return std::make_shared<ChatSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // SSL server port
    int port = 2222;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "SSL server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new SSL chat server
    auto server = std::make_shared<ChatServer>(service, context, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->Multicast(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: SSL chat client
Here comes the example of the SSL chat client. It connects to the SSL chat
server and allows to send message to it and receive new messages.

This example is very similar to the TCP one except the code that prepares SSL
context and handshake handler.

```c++
#include "server/asio/ssl_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::Asio::SSLClient
{
public:
    ChatClient(std::shared_ptr<CppServer::Asio::Service> service, std::shared_ptr<CppServer::Asio::SSLContext> context, const std::string& address, int port)
        : CppServer::Asio::SSLClient(service, context, address, port)
    {
        _stop = false;
    }

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onConnected() override
    {
        std::cout << "Chat SSL client connected a new session with Id " << id() << std::endl;
    }

    void onHandshaked() override
    {
        std::cout << "Chat SSL client handshaked a new session with Id " << id() << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Chat SSL client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop;
};

int main(int argc, char** argv)
{
    // SSL server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // SSL server port
    int port = 2222;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "SSL server address: " << address << std::endl;
    std::cout << "SSL server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->load_verify_file("../tools/certificates/ca.pem");

    // Create a new SSL chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->DisconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: UDP echo server
Here comes the example of the UDP echo server. It receives a datagram mesage
from any UDP client and resend it back without any changes.

```c++
#include "server/asio/udp_server.h"
#include "threads/thread.h"

#include <iostream>

class EchoServer : public CppServer::Asio::UDPServer
{
public:
    using CppServer::Asio::UDPServer::UDPServer;

protected:
    void onStarted() override
    {
        // Start receive datagrams
        ReceiveAsync();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Echo the message back to the sender
        SendAsync(endpoint, message);
    }

    void onSent(const asio::ip::udp::endpoint& endpoint, size_t sent) override
    {
        // Continue receive datagrams
        ReceiveAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Echo UDP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP server port
    int port = 3333;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "UDP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP echo server
    auto server = std::make_shared<EchoServer>(service, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: UDP echo client
Here comes the example of the UDP echo client. It sends user datagram message
to UDP server and listen for response.

```c++
#include "server/asio/udp_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class EchoClient : public CppServer::Asio::UDPClient
{
public:
    EchoClient(std::shared_ptr<CppServer::Asio::Service> service, const std::string& address, int port)
        : CppServer::Asio::UDPClient(service, address, port)
    {
        _stop = false;
    }

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onConnected() override
    {
        std::cout << "Echo UDP client connected a new session with Id " << id() << std::endl;

        // Start receive datagrams
        ReceiveAsync();
    }

    void onDisconnected() override
    {
        std::cout << "Echo UDP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;

        // Continue receive datagrams
        ReceiveAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Echo UDP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop;
};

int main(int argc, char** argv)
{
    // UDP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // UDP server port
    int port = 3333;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "UDP server address: " << address << std::endl;
    std::cout << "UDP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP echo client
    auto client = std::make_shared<EchoClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->DisconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the echo server
        client->SendSync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: UDP multicast server
Here comes the example of the UDP multicast server. It use multicast IP address
to multicast datagram messages to all client that joined corresponding UDP
multicast group.

```c++
#include "server/asio/udp_server.h"
#include "threads/thread.h"

#include <iostream>

class MulticastServer : public CppServer::Asio::UDPServer
{
public:
    using CppServer::Asio::UDPServer::UDPServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Multicast UDP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP multicast address
    std::string multicast_address = "239.255.0.1";
    if (argc > 1)
        multicast_address = argv[1];

    // UDP multicast port
    int multicast_port = 3334;
    if (argc > 2)
        multicast_port = std::atoi(argv[2]);

    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast server
    auto server = std::make_shared<MulticastServer>(service, 0);

    // Start the multicast server
    std::cout << "Server starting...";
    server->Start(multicast_address, multicast_port);
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->MulticastSync(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: UDP multicast client
Here comes the example of the UDP multicast client. It use multicast IP address
and joins UDP multicast group in order to receive multicasted datagram messages
from UDP server.

```c++
#include "server/asio/udp_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class MulticastClient : public CppServer::Asio::UDPClient
{
public:
    MulticastClient(std::shared_ptr<CppServer::Asio::Service> service, const std::string& address, const std::string& multicast, int port)
        : CppServer::Asio::UDPClient(service, address, port),
          _multicast(multicast)
    {
        _stop = false;
    }

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onConnected() override
    {
        std::cout << "Multicast UDP client connected a new session with Id " << id() << std::endl;

        // Join UDP multicast group
        JoinMulticastGroupAsync(_multicast);

        // Start receive datagrams
        ReceiveAsync();
    }

    void onDisconnected() override
    {
        std::cout << "Multicast UDP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;

        // Continue receive datagrams
        ReceiveAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Multicast UDP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop;
    std::string _multicast;
};

int main(int argc, char** argv)
{
    // UDP listen address
    std::string listen_address = "0.0.0.0";
    if (argc > 1)
        listen_address = argv[1];

    // UDP multicast address
    std::string multicast_address = "239.255.0.1";
    if (argc > 2)
        multicast_address = argv[2];

    // UDP multicast port
    int multicast_port = 3334;
    if (argc > 3)
        multicast_port = std::atoi(argv[3]);

    std::cout << "UDP listen address: " << listen_address << std::endl;
    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast client
    auto client = std::make_shared<MulticastClient>(service, listen_address, multicast_address, multicast_port);
    client->SetupMulticast(true);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Disconnect the client
        if (line == "!")
        {
            std::cout << "Client disconnecting...";
            client->DisconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: Simple protocol
Simple protocol is defined in [simple.fbe](https://github.com/chronoxor/CppServer/blob/master/proto/simple.fbe) file:

```proto
/*
   Simple Fast Binary Encoding protocol for CppServer
   https://github.com/chronoxor/FastBinaryEncoding

   Generate protocol command: fbec --cpp --proto --input=simple.fbe --output=.
*/

// Domain declaration
domain com.chronoxor

// Package declaration
package simple

// Protocol version
version 1.0

// Simple request message
[request]
[response(SimpleResponse)]
[reject(SimpleReject)]
message SimpleRequest
{
    // Request Id
    uuid [id] = uuid1;
    // Request message
    string Message;
}

// Simple response
message SimpleResponse
{
    // Response Id
    uuid [id] = uuid1;
    // Calculated message hash
    uint32 Hash;
}

// Simple reject
message SimpleReject
{
    // Reject Id
    uuid [id] = uuid1;
    // Error message
    string Error;
}

// Simple notification
message SimpleNotify
{
    // Server notification
    string Notification;
}

// Disconnect request message
[request]
message DisconnectRequest
{
    // Request Id
    uuid [id] = uuid1;
}
```

## Example: Simple protocol server
Here comes the example of  the  simple  protocol  server.  It  process  client
requests, answer with corresponding responses and  send  server  notifications
back to clients.

```c++
#include "asio_service.h"

#include "server/asio/tcp_server.h"

#include "../proto/simple_protocol.h"

#include <iostream>

class SimpleProtoSession : public CppServer::Asio::TCPSession, public FBE::simple::Sender, public FBE::simple::Receiver
{
public:
    using CppServer::Asio::TCPSession::TCPSession;

protected:
    void onConnected() override
    {
        std::cout << "Simple protocol session with Id " << id() << " connected!" << std::endl;

        // Send invite notification
        simple::SimpleNotify notify;
        notify.Notification = "Hello from Simple protocol server! Please send a message or '!' to disconnect the client!";
        send(notify);
    }

    void onDisconnected() override
    {
        std::cout << "Simple protocol session with Id " << id() << " disconnected!" << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol handlers
    void onReceive(const ::simple::DisconnectRequest& request) override { Disconnect(); }
    void onReceive(const ::simple::SimpleRequest& request) override
    {
        std::cout << "Received: " << request << std::endl;

        // Validate request
        if (request.Message.empty())
        {
            // Send reject
            simple::SimpleReject reject;
            reject.id = request.id;
            reject.Error = "Request message is empty!";
            send(reject);
            return;
        }

        static std::hash<std::string> hasher;

        // Send response
        simple::SimpleResponse response;
        response.id = request.id;
        response.Hash = (uint32_t)hasher(request.Message);
        send(response);
    }

    // Protocol implementation
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }
};

class SimpleProtoServer : public CppServer::Asio::TCPServer, public FBE::simple::Sender
{
public:
    using CppServer::Asio::TCPServer::TCPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override
    {
        return std::make_shared<SimpleProtoSession>(server);
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol implementation
    size_t onSend(const void* data, size_t size) override { Multicast(data, size); return size; }
};

int main(int argc, char** argv)
{
    // Simple protocol server port
    int port = 4444;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "Simple protocol server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new simple protocol server
    auto server = std::make_shared<SimpleProtoServer>(service, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin notification to all sessions
        simple::SimpleNotify notify;
        notify.Notification = "(admin) " + line;
        server->send(notify);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: Simple protocol client
Here comes the example of the simple  protocol  client.  It  connects  to  the
simple protocol  server  and  allows  to  send  requests  to  it  and  receive
corresponding responses.

```c++
#include "asio_service.h"

#include "server/asio/tcp_client.h"
#include "threads/thread.h"

#include "../proto/simple_protocol.h"

#include <atomic>
#include <iostream>

class SimpleProtoClient : public CppServer::Asio::TCPClient, public FBE::simple::Client
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

    void DisconnectAndStop()
    {
        _stop = true;
        DisconnectAsync();
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onConnected() override
    {
        std::cout << "Simple protocol client connected a new session with Id " << id() << std::endl;

        // Reset FBE protocol buffers
        reset();
    }

    void onDisconnected() override
    {
        std::cout << "Simple protocol client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Simple protocol client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

    // Protocol handlers
    void onReceive(const ::simple::DisconnectRequest& request) override { Client::onReceive(request); std::cout << "Received: " << request << std::endl; DisconnectAsync(); }
    void onReceive(const ::simple::SimpleResponse& response) override { Client::onReceive(response); std::cout << "Received: " << response << std::endl; }
    void onReceive(const ::simple::SimpleReject& reject) override { Client::onReceive(reject); std::cout << "Received: " << reject << std::endl; }
    void onReceive(const ::simple::SimpleNotify& notify) override { Client::onReceive(notify); std::cout << "Received: " << notify << std::endl; }

    // Protocol implementation
    void onReceived(const void* buffer, size_t size) override { receive(buffer, size); }
    size_t onSend(const void* data, size_t size) override { return SendAsync(data, size) ? size : 0; }

private:
    std::atomic<bool> _stop{false};
};

int main(int argc, char** argv)
{
    // TCP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // Simple protocol server port
    int port = 4444;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "Simple protocol server address: " << address << std::endl;
    std::cout << "Simple protocol server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new simple protocol client
    auto client = std::make_shared<SimpleProtoClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Reconnect the client
        if (line == "!")
        {
            std::cout << "Client reconnecting...";
            client->IsConnected() ? client->ReconnectAsync() : client->ConnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send request to the simple protocol server
        simple::SimpleRequest request;
        request.Message = line;
        auto response = client->request(request).get();

        // Show string hash calculation result
        std::cout << "Hash of '" << line << "' = " << std::format("0x{:8X}", response.Hash) << std::endl;
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTP server
Here comes the example of the HTTP cache server. It allows to manipulate
cache data with HTTP methods (GET, POST, PUT and DELETE).

Use the following link to open [Swagger OpenAPI](https://swagger.io/specification/) iterative documentation: http://localhost:8080/api/index.html

![OpenAPI-HTTP](https://github.com/chronoxor/CppServer/raw/master/images/openapi-http.png)

```c++
#include "server/http/http_server.h"
#include "string/string_utils.h"
#include "utility/singleton.h"

#include <iostream>
#include <map>
#include <mutex>

class Cache : public CppCommon::Singleton<Cache>
{
   friend CppCommon::Singleton<Cache>;

public:
    std::string GetAllCache()
    {
        std::scoped_lock locker(_cache_lock);
        std::string result;
        result += "[\n";
        for (const auto& item : _cache)
        {
            result += "  {\n";
            result += "    \"key\": \"" + item.first + "\",\n";
            result += "    \"value\": \"" + item.second + "\",\n";
            result += "  },\n";
        }
        result += "]\n";
        return result;
    }

    bool GetCacheValue(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            return true;
        }
        else
            return false;
    }

    void PutCacheValue(std::string_view key, std::string_view value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.emplace(key, value);
        if (!it.second)
            it.first->second = value;
    }

    bool DeleteCacheValue(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            _cache.erase(it);
            return true;
        }
        else
            return false;
    }

private:
    std::mutex _cache_lock;
    std::map<std::string, std::string, std::less<>> _cache;
};

class HTTPCacheSession : public CppServer::HTTP::HTTPSession
{
public:
    using CppServer::HTTP::HTTPSession::HTTPSession;

protected:
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override
    {
        // Show HTTP request content
        std::cout << std::endl << request;

        // Process HTTP request methods
        if (request.method() == "HEAD")
            SendResponseAsync(response().MakeHeadResponse());
        else if (request.method() == "GET")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            if (key.empty())
            {
                // Response with all cache values
                SendResponseAsync(response().MakeGetResponse(Cache::GetInstance().GetAllCache(), "application/json; charset=UTF-8"));
            }
            // Get the cache value by the given key
            else if (Cache::GetInstance().GetCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Required cache value was not found for the key: " + key));
        }
        else if ((request.method() == "POST") || (request.method() == "PUT"))
        {
            std::string key(request.url());
            std::string value(request.body());

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Put the cache value
            Cache::GetInstance().PutCacheValue(key, value);

            // Response with the cache value
            SendResponseAsync(response().MakeOKResponse());
        }
        else if (request.method() == "DELETE")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Delete the cache value
            if (Cache::GetInstance().DeleteCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Deleted cache value was not found for the key: " + key));
        }
        else if (request.method() == "OPTIONS")
            SendResponseAsync(response().MakeOptionsResponse());
        else if (request.method() == "TRACE")
            SendResponseAsync(response().MakeTraceResponse(request.cache()));
        else
            SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP method: " + std::string(request.method())));
    }

    void onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) override
    {
        std::cout << "Request error: " << error << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class HTTPCacheServer : public CppServer::HTTP::HTTPServer
{
public:
    using CppServer::HTTP::HTTPServer::HTTPServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override
    {
        return std::make_shared<HTTPCacheSession>(std::dynamic_pointer_cast<CppServer::HTTP::HTTPServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // HTTP server port
    int port = 8080;
    if (argc > 1)
        port = std::atoi(argv[1]);
    // HTTP server content path
    std::string www = "../www/api";
    if (argc > 2)
        www = argv[2];

    std::cout << "HTTP server port: " << port << std::endl;
    std::cout << "HTTP server static content path: " << www << std::endl;
    std::cout << "HTTP server website: " << "http://localhost:" << port << "/api/index.html" << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP server
    auto server = std::make_shared<HTTPCacheServer>(service, port);
    server->AddStaticContent(www, "/api");

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTP client
Here comes the example of the HTTP client. It allows to send HTTP requests
(GET, POST, PUT and DELETE) and receive HTTP responses.

```c++
#include "server/http/http_client.h"
#include "string/string_utils.h"

#include <iostream>

int main(int argc, char** argv)
{
    // HTTP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTP server address: " << address << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP client
    auto client = std::make_shared<CppServer::HTTP::HTTPClientEx>(service, address, "http");

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    try
    {
        // Perform text input
        std::string line;
        while (getline(std::cin, line))
        {
            if (line.empty())
                break;

            // Reconnect the client
            if (line == "!")
            {
                std::cout << "Client reconnecting...";
                client->ReconnectAsync();
                std::cout << "Done!" << std::endl;
                continue;
            }

            auto commands = CppCommon::StringUtils::Split(line, ' ', true);
            if (commands.size() < 2)
            {
                std::cout << "HTTP method and URL must be entered!" << std::endl;
                continue;
            }

            if (CppCommon::StringUtils::ToUpper(commands[0]) == "HEAD")
            {
                auto response = client->SendHeadRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "GET")
            {
                auto response = client->SendGetRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "POST")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPostRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "PUT")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPutRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "DELETE")
            {
                auto response = client->SendDeleteRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "OPTIONS")
            {
                auto response = client->SendOptionsRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "TRACE")
            {
                auto response = client->SendTraceRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else
                std::cout << "Unknown HTTP method: " << commands[0] << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTPS server
Here comes the example of the HTTPS cache server. It allows to manipulate
cache data with HTTP methods (GET, POST, PUT and DELETE) with secured
transport protocol.

Use the following link to open [Swagger OpenAPI](https://swagger.io/specification/) iterative documentation: https://localhost:8443/api/index.html

![OpenAPI-HTTPS](https://github.com/chronoxor/CppServer/raw/master/images/openapi-https.png)

```c++
#include "server/http/https_server.h"
#include "string/string_utils.h"
#include "utility/singleton.h"

#include <iostream>
#include <map>
#include <mutex>

class Cache : public CppCommon::Singleton<Cache>
{
   friend CppCommon::Singleton<Cache>;

public:
    std::string GetAllCache()
    {
        std::scoped_lock locker(_cache_lock);
        std::string result;
        result += "[\n";
        for (const auto& item : _cache)
        {
            result += "  {\n";
            result += "    \"key\": \"" + item.first + "\",\n";
            result += "    \"value\": \"" + item.second + "\",\n";
            result += "  },\n";
        }
        result += "]\n";
        return result;
    }

    bool GetCacheValue(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            return true;
        }
        else
            return false;
    }

    void PutCacheValue(std::string_view key, std::string_view value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.emplace(key, value);
        if (!it.second)
            it.first->second = value;
    }

    bool DeleteCacheValue(std::string_view key, std::string& value)
    {
        std::scoped_lock locker(_cache_lock);
        auto it = _cache.find(key);
        if (it != _cache.end())
        {
            value = it->second;
            _cache.erase(it);
            return true;
        }
        else
            return false;
    }

private:
    std::mutex _cache_lock;
    std::map<std::string, std::string, std::less<>> _cache;
};

class HTTPSCacheSession : public CppServer::HTTP::HTTPSSession
{
public:
    using CppServer::HTTP::HTTPSSession::HTTPSSession;

protected:
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override
    {
        // Show HTTP request content
        std::cout << std::endl << request;

        // Process HTTP request methods
        if (request.method() == "HEAD")
            SendResponseAsync(response().MakeHeadResponse());
        else if (request.method() == "GET")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            if (key.empty())
            {
                // Response with all cache values
                SendResponseAsync(response().MakeGetResponse(Cache::GetInstance().GetAllCache(), "application/json; charset=UTF-8"));
            }
            // Get the cache value by the given key
            else if (Cache::GetInstance().GetCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Required cache value was not found for the key: " + key));
        }
        else if ((request.method() == "POST") || (request.method() == "PUT"))
        {
            std::string key(request.url());
            std::string value(request.body());

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Put the cache value
            Cache::GetInstance().PutCacheValue(key, value);

            // Response with the cache value
            SendResponseAsync(response().MakeOKResponse());
        }
        else if (request.method() == "DELETE")
        {
            std::string key(request.url());
            std::string value;

            // Decode the key value
            key = CppCommon::Encoding::URLDecode(key);
            CppCommon::StringUtils::ReplaceFirst(key, "/api/cache", "");
            CppCommon::StringUtils::ReplaceFirst(key, "?key=", "");

            // Delete the cache value
            if (Cache::GetInstance().DeleteCacheValue(key, value))
            {
                // Response with the cache value
                SendResponseAsync(response().MakeGetResponse(value));
            }
            else
                SendResponseAsync(response().MakeErrorResponse(404, "Deleted cache value was not found for the key: " + key));
        }
        else if (request.method() == "OPTIONS")
            SendResponseAsync(response().MakeOptionsResponse());
        else if (request.method() == "TRACE")
            SendResponseAsync(response().MakeTraceResponse(request.cache()));
        else
            SendResponseAsync(response().MakeErrorResponse("Unsupported HTTP method: " + std::string(request.method())));
    }

    void onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) override
    {
        std::cout << "Request error: " << error << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTPS session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class HTTPSCacheServer : public CppServer::HTTP::HTTPSServer
{
public:
    using CppServer::HTTP::HTTPSServer::HTTPSServer;

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer>& server) override
    {
        return std::make_shared<HTTPSCacheSession>(std::dynamic_pointer_cast<CppServer::HTTP::HTTPSServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTPS server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // HTTPS server port
    int port = 8443;
    if (argc > 1)
        port = std::atoi(argv[1]);
    // HTTPS server content path
    std::string www = "../www/api";
    if (argc > 2)
        www = argv[2];

    std::cout << "HTTPS server port: " << port << std::endl;
    std::cout << "HTTPS server static content path: " << www << std::endl;
    std::cout << "HTTPS server website: " << "https://localhost:" << port << "/api/index.html" << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new HTTPS server
    auto server = std::make_shared<HTTPSCacheServer>(service, context, port);
    server->AddStaticContent(www, "/api");

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTPS client
Here comes the example of the HTTPS client. It allows to send HTTP requests
(GET, POST, PUT and DELETE) and receive HTTP responses with secured
transport protocol.

```c++
#include "server/http/https_client.h"
#include "string/string_utils.h"

#include <iostream>

int main(int argc, char** argv)
{
    // HTTP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTPS server address: " << address << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->load_verify_file("../tools/certificates/ca.pem");

    // Create a new HTTP client
    auto client = std::make_shared<CppServer::HTTP::HTTPSClientEx>(service, context, address, "https");

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    try
    {
        // Perform text input
        std::string line;
        while (getline(std::cin, line))
        {
            if (line.empty())
                break;

            // Reconnect the client
            if (line == "!")
            {
                std::cout << "Client reconnecting...";
                client->ReconnectAsync();
                std::cout << "Done!" << std::endl;
                continue;
            }

            auto commands = CppCommon::StringUtils::Split(line, ' ', true);
            if (commands.size() < 2)
            {
                std::cout << "HTTP method and URL must be entered!" << std::endl;
                continue;
            }

            if (CppCommon::StringUtils::ToUpper(commands[0]) == "HEAD")
            {
                auto response = client->SendHeadRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "GET")
            {
                auto response = client->SendGetRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "POST")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPostRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "PUT")
            {
                if (commands.size() < 3)
                {
                    std::cout << "HTTP method, URL and body must be entered!" << std::endl;
                    continue;
                }
                auto response = client->SendPutRequest(commands[1], commands[2]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "DELETE")
            {
                auto response = client->SendDeleteRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "OPTIONS")
            {
                auto response = client->SendOptionsRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else if (CppCommon::StringUtils::ToUpper(commands[0]) == "TRACE")
            {
                auto response = client->SendTraceRequest(commands[1]).get();
                std::cout << response << std::endl;
            }
            else
                std::cout << "Unknown HTTP method: " << commands[0] << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: WebSocket chat server
Here comes the example of the WebSocket chat server. It handles multiple
WebSocket client sessions and multicast received message from any session
to all ones. Also it is possible to send admin message directly from the
server.

Use the following link to open WebSocket chat server example: http://localhost:8080/chat/index.html

![ws-chat](https://github.com/chronoxor/CppServer/raw/master/images/ws-chat.png)

```c++
#include "server/ws/ws_server.h"

#include <iostream>

class ChatSession : public CppServer::WS::WSSession
{
public:
    using CppServer::WS::WSSession::WSSession;

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override
    {
        std::cout << "Chat WebSocket session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket chat! Please send a message or '!' to disconnect the client!");
        SendTextAsync(message);
    }

    void onWSDisconnected() override
    {
        std::cout << "Chat WebSocket session with Id " << id() << " disconnected!" << std::endl;
    }

    void onWSReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        std::dynamic_pointer_cast<CppServer::WS::WSServer>(server())->MulticastText(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Close(1000);
    }

    void onWSPing(const void* buffer, size_t size) override
    {
        SendPongAsync(buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatServer : public CppServer::WS::WSServer
{
public:
    using CppServer::WS::WSServer::WSServer;

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(std::shared_ptr<CppServer::Asio::TCPServer> server) override
    {
        return std::make_shared<ChatSession>(std::dynamic_pointer_cast<CppServer::WS::WSServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket server port
    int port = 8080;
    if (argc > 1)
        port = std::atoi(argv[1]);
    // WebSocket server content path
    std::string www = "../www/ws";
    if (argc > 2)
        www = argv[2];

    std::cout << "WebSocket server port: " << port << std::endl;
    std::cout << "WebSocket server static content path: " << www << std::endl;
    std::cout << "WebSocket server website: " << "http://localhost:" << port << "/chat/index.html" << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new WebSocket chat server
    auto server = std::make_shared<ChatServer>(service, port);
    server->AddStaticContent(www, "/chat");

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->MulticastText(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: WebSocket chat client
Here comes the example of the WebSocket chat client. It connects to the
WebSocket chat server and allows to send message to it and receive new
messages.

```c++
#include "server/ws/ws_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::WS::WSClient
{
public:
    using CppServer::WS::WSClient::WSClient;

    void DisconnectAndStop()
    {
        _stop = true;
        CloseAsync(1000);
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onWSConnecting(CppServer::HTTP::HTTPRequest& request) override
    {
        request.SetBegin("GET", "/");
        request.SetHeader("Host", "localhost");
        request.SetHeader("Origin", "http://localhost");
        request.SetHeader("Upgrade", "websocket");
        request.SetHeader("Connection", "Upgrade");
        request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(ws_nonce()));
        request.SetHeader("Sec-WebSocket-Protocol", "chat, superchat");
        request.SetHeader("Sec-WebSocket-Version", "13");
    }

    void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override
    {
        std::cout << "Chat WebSocket client connected a new session with Id " << id() << std::endl;
    }

    void onWSDisconnected() override
    {
        std::cout << "Chat WebSocket client disconnected a session with Id " << id() << std::endl;
    }

    void onWSReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onWSPing(const void* buffer, size_t size) override
    {
        SendPongAsync(buffer, size);
    }

    void onDisconnected() override
    {
        WSClient::onDisconnected();

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop{false};
};

int main(int argc, char** argv)
{
    // WebSocket server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // WebSocket server port
    int port = 8080;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "WebSocket server address: " << address << std::endl;
    std::cout << "WebSocket server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new WebSocket chat client
    auto client = std::make_shared<ChatClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Reconnect the client
        if (line == "!")
        {
            std::cout << "Client reconnecting...";
            client->ReconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendTextAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: WebSocket secure chat server
Here comes the example of the WebSocket secure chat server. It handles
multiple WebSocket secure client sessions and multicast received message
from any session to all ones. Also it is possible to send admin message
directly from the server.

This example is very similar to the WebSocket one except the code that
prepares WebSocket secure context and handshake handler.

Use the following link to open WebSocket secure chat server example: https://localhost:8443/chat/index.html

![wss-chat](https://github.com/chronoxor/CppServer/raw/master/images/wss-chat.png)

```c++
#include "server/ws/wss_server.h"

#include <iostream>

class ChatSession : public CppServer::WS::WSSSession
{
public:
    using CppServer::WS::WSSSession::WSSSession;

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override
    {
        std::cout << "Chat WebSocket secure session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket secure chat! Please send a message or '!' to disconnect the client!");
        SendTextAsync(message);
    }

    void onWSDisconnected() override
    {
        std::cout << "Chat WebSocket secure session with Id " << id() << " disconnected!" << std::endl;
    }

    void onWSReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        std::dynamic_pointer_cast<CppServer::WS::WSSServer>(server())->MulticastText(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Close(1000);
    }

    void onWSPing(const void* buffer, size_t size) override
    {
        SendPongAsync(buffer, size);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket secure session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatServer : public CppServer::WS::WSSServer
{
public:
    using CppServer::WS::WSSServer::WSSServer;

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(std::shared_ptr<CppServer::Asio::SSLServer> server) override
    {
        return std::make_shared<ChatSession>(std::dynamic_pointer_cast<CppServer::WS::WSSServer>(server));
    }

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket secure server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket secure server port
    int port = 8443;
    if (argc > 1)
        port = std::atoi(argv[1]);
    // WebSocket secure server content path
    std::string www = "../www/wss";
    if (argc > 2)
        www = argv[2];

    std::cout << "WebSocket secure server port: " << port << std::endl;
    std::cout << "WebSocket secure server static content path: " << www << std::endl;
    std::cout << "WebSocket server website: " << "https://localhost:" << port << "/chat/index.html" << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_password_callback([](size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new WebSocket secure chat server
    auto server = std::make_shared<ChatServer>(service, context, port);
    server->AddStaticContent(www, "/chat");

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Multicast admin message to all sessions
        line = "(admin) " + line;
        server->MulticastText(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: WebSocket secure chat client
Here comes the example of the WebSocket secure chat client. It connects to
the WebSocket secure chat server and allows to send message to it and receive
new messages.

This example is very similar to the WebSocket one except the code that
prepares WebSocket secure context and handshake handler.

```c++
#include "server/ws/wss_client.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>

class ChatClient : public CppServer::WS::WSSClient
{
public:
    using CppServer::WS::WSSClient::WSSClient;

    void DisconnectAndStop()
    {
        _stop = true;
        CloseAsync(1000);
        while (IsConnected())
            CppCommon::Thread::Yield();
    }

protected:
    void onWSConnecting(CppServer::HTTP::HTTPRequest& request) override
    {
        request.SetBegin("GET", "/");
        request.SetHeader("Host", "localhost");
        request.SetHeader("Origin", "https://localhost");
        request.SetHeader("Upgrade", "websocket");
        request.SetHeader("Connection", "Upgrade");
        request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(ws_nonce()));
        request.SetHeader("Sec-WebSocket-Protocol", "chat, superchat");
        request.SetHeader("Sec-WebSocket-Version", "13");
    }

    void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override
    {
        std::cout << "Chat WebSocket secure client connected a new session with Id " << id() << std::endl;
    }

    void onWSDisconnected() override
    {
        std::cout << "Chat WebSocket secure client disconnected a session with Id " << id() << std::endl;
    }

    void onWSReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onWSPing(const void* buffer, size_t size) override
    {
        SendPongAsync(buffer, size);
    }

    void onDisconnected() override
    {
        WSSClient::onDisconnected();

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        if (!_stop)
            ConnectAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket secure client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }

private:
    std::atomic<bool> _stop{false};
};

int main(int argc, char** argv)
{
    // WebSocket server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // WebSocket server port
    int port = 8443;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "WebSocket secure server address: " << address << std::endl;
    std::cout << "WebSocket secure server port: " << port << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    auto context = std::make_shared<CppServer::Asio::SSLContext>(asio::ssl::context::tlsv12);
    context->set_default_verify_paths();
    context->set_root_certs();
    context->set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    context->load_verify_file("../tools/certificates/ca.pem");

    // Create a new WebSocket chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->ConnectAsync();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Reconnect the client
        if (line == "!")
        {
            std::cout << "Client reconnecting...";
            client->ReconnectAsync();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->SendTextAsync(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->DisconnectAndStop();
    std::cout << "Done!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

# Performance

Here comes several communication scenarios with timing measurements.

Benchmark environment is the following:
```
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.623 GiB

OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
```

## Benchmark: Round-Trip

![Round-trip](https://github.com/chronoxor/CppServer/raw/master/images/round-trip.png)

This scenario sends lots of messages from several clients to a server.
The server responses to each message and resend the similar response to
the client. The benchmark measures total round-trip time to send all
messages and receive all responses, messages & data throughput, count
of errors.

### TCP echo server

* [cppserver-performance-tcp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_server.cpp)
* [cppserver-performance-tcp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.001 s
Total data: 1.692 GiB
Total messages: 56261685
Data throughput: 171.693 MiB/s
Message latency: 177 ns
Message throughput: 5625528 msg/s
```

* [cppserver-performance-tcp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_server.cpp)
* [cppserver-performance-tcp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.007 s
Total data: 1.151 GiB
Total messages: 38503396
Data throughput: 117.423 MiB/s
Message latency: 259 ns
Message throughput: 3847402 msg/s
```

### SSL echo server

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.012 s
Total data: 296.350 MiB
Total messages: 9710535
Data throughput: 29.612 MiB/s
Message latency: 1.031 mcs
Message throughput: 969878 msg/s
```

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.341 s
Total data: 390.660 MiB
Total messages: 12800660
Data throughput: 37.792 MiB/s
Message latency: 807 ns
Message throughput: 1237782 msg/s
```

### UDP echo server

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 3333
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.002 s
Total data: 46.032 MiB
Total messages: 1508355
Data throughput: 4.616 MiB/s
Message latency: 6.631 mcs
Message throughput: 150801 msg/s
```

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 3333
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.152 s
Total data: 32.185 MiB
Total messages: 1054512
Data throughput: 3.173 MiB/s
Message latency: 9.627 mcs
Message throughput: 103867 msg/s
```

### Simple protocol server

* [cppserver-performance-proto_server](https://github.com/chronoxor/CppServer/blob/master/performance/proto_server.cpp)
* [cppserver-performance-proto_client](https://github.com/chronoxor/CppServer/blob/master/performance/proto_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 4444
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.002 s
Total data: 497.096 MiB
Total messages: 16288783
Data throughput: 49.715 MiB/s
Message latency: 614 ns
Message throughput: 1628542 msg/s
```

* [cppserver-performance-proto_server](https://github.com/chronoxor/CppServer/blob/master/performance/proto_server.cpp)
* [cppserver-performance-proto_client](https://github.com/chronoxor/CppServer/blob/master/performance/proto_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 4444
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.066 s
Total data: 997.384 MiB
Total messages: 32681995
Data throughput: 99.078 MiB/s
Message latency: 308 ns
Message throughput: 3246558 msg/s
```

### WebSocket echo server

* [cppserver-performance-ws_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ws_echo_server.cpp)
* [cppserver-performance-ws_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ws_echo_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 8080
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 9.994 s
Total data: 48.958 MiB
Total messages: 1603548
Data throughput: 4.918 MiB/s
Message latency: 6.232 mcs
Message throughput: 160448 msg/s
```

* [cppserver-performance-ws_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ws_echo_server.cpp)
* [cppserver-performance-ws_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ws_echo_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 8080
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 11.402 s
Total data: 206.827 MiB
Total messages: 6776702
Data throughput: 18.140 MiB/s
Message latency: 1.682 mcs
Message throughput: 594328 msg/s
```

### WebSocket secure echo server

* [cppserver-performance-wss_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/wss_echo_server.cpp)
* [cppserver-performance-wss_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/wss_echo_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 8443
Working threads: 1
Working clients: 1
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.001 s
Total data: 62.068 MiB
Total messages: 2033811
Data throughput: 6.210 MiB/s
Message latency: 4.917 mcs
Message throughput: 203343 msg/s
```

* [cppserver-performance-wss_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/wss_echo_server.cpp)
* [cppserver-performance-wss_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/wss_echo_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 8443
Working threads: 4
Working clients: 100
Working messages: 1000
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.011 s
Total data: 249.1023 MiB
Total messages: 8191971
Data throughput: 24.993 MiB/s
Message latency: 1.222 mcs
Message throughput: 818230 msg/s
```

## Benchmark: Multicast

![Multicast](https://github.com/chronoxor/CppServer/raw/master/images/multicast.png)

In this scenario server multicasts messages to all connected clients.
The benchmark counts total messages received by all clients for all
the working time and measures messages & data throughput, count
of errors.

### TCP multicast server

* [cppserver-performance-tcp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_server.cpp)
* [cppserver-performance-tcp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 1
Working clients: 1
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.001 s
Total data: 1.907 GiB
Total messages: 63283367
Data throughput: 193.103 MiB/s
Message latency: 158 ns
Message throughput: 6327549 msg/s
```

* [cppserver-performance-tcp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_server.cpp)
* [cppserver-performance-tcp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 4
Working clients: 100
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.006 s
Total data: 1.1006 GiB
Total messages: 66535013
Data throughput: 202.930 MiB/s
Message latency: 150 ns
Message throughput: 6648899 msg/s
```

### SSL multicast server

* [cppserver-performance-ssl_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_server.cpp)
* [cppserver-performance-ssl_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 1
Working clients: 1
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.014 s
Total data: 1.535 GiB
Total messages: 51100073
Data throughput: 155.738 MiB/s
Message latency: 195 ns
Message throughput: 5102683 msg/s
```

* [cppserver-performance-ssl_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_server.cpp)
* [cppserver-performance-ssl_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 4
Working clients: 100
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.691 s
Total data: 1.878 GiB
Total messages: 62334478
Data throughput: 177.954 MiB/s
Message latency: 171 ns
Message throughput: 5830473 msg/s
```

### UDP multicast server

* [cppserver-performance-udp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_server.cpp)
* [cppserver-performance-udp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_client.cpp) --clients 1 --threads 1

```
Server address: 239.255.0.1
Server port: 3333
Working threads: 1
Working clients: 1
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.002 s
Total data: 23.777 MiB
Total messages: 778555
Data throughput: 2.384 MiB/s
Message latency: 12.847 mcs
Message throughput: 77833 msg/s
```

* [cppserver-performance-udp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_server.cpp)
* [cppserver-performance-udp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_client.cpp) --clients 100 --threads 4

```
Server address: 239.255.0.1
Server port: 3333
Working threads: 4
Working clients: 100
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.004 s
Total data: 52.457 MiB
Total messages: 1718575
Data throughput: 5.248 MiB/s
Message latency: 5.821 mcs
Message throughput: 171784 msg/s
```

### WebSocket multicast server

* [cppserver-performance-ws_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ws_multicast_server.cpp)
* [cppserver-performance-ws_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ws_multicast_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 8080
Working threads: 1
Working clients: 1
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.001 s
Total data: 960.902 MiB
Total messages: 31486166
Data throughput: 96.075 MiB/s
Message latency: 317 ns
Message throughput: 3148135 msg/s
```

* [cppserver-performance-ws_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ws_multicast_server.cpp)
* [cppserver-performance-ws_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ws_multicast_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 8080
Working threads: 4
Working clients: 100
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.020 s
Total data: 986.489 MiB
Total messages: 32324898
Data throughput: 98.459 MiB/s
Message latency: 309 ns
Message throughput: 3225965 msg/s
```

### WebSocket secure multicast server

* [cppserver-performance-wss_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/wss_multicast_server.cpp)
* [cppserver-performance-wss_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/wss_multicast_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 8443
Working threads: 1
Working clients: 1
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.002 s
Total data: 1.041 GiB
Total messages: 34903186
Data throughput: 106.505 MiB/s
Message latency: 286 ns
Message throughput: 3489578 msg/s
```

* [cppserver-performance-wss_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/wss_multicast_server.cpp)
* [cppserver-performance-wss_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/wss_multicast_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 8443
Working threads: 4
Working clients: 100
Message size: 32
Seconds to benchmarking: 10

Errors: 0

Total time: 10.013 s
Total data: 1.569 GiB
Total messages: 52225588
Data throughput: 159.172 MiB/s
Message latency: 191 ns
Message throughput: 5215639 msg/s
```

## Benchmark: Web Server

### HTTP Trace server

* [cppserver-performance-http_trace_server](https://github.com/chronoxor/CppServer/blob/master/performance/http_trace_server.cpp)
* [cppserver-performance-http_trace_client](https://github.com/chronoxor/CppServer/blob/master/performance/http_trace_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 80
Working threads: 1
Working clients: 1
Working messages: 1
Seconds to benchmarking: 10

Errors: 0

Total time: 10.001 s
Total data: 58.476 MiB
Total messages: 578353
Data throughput: 5.865 MiB/s
Message latency: 17.293 mcs
Message throughput: 57825 msg/s
```

* [cppserver-performance-http_trace_server](https://github.com/chronoxor/CppServer/blob/master/performance/http_trace_server.cpp)
* [cppserver-performance-http_trace_client](https://github.com/chronoxor/CppServer/blob/master/performance/http_trace_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 80
Working threads: 4
Working clients: 100
Working messages: 1
Seconds to benchmarking: 10

Errors: 0

Total time: 10.006 s
Total data: 310.730 MiB
Total messages: 3073650
Data throughput: 31.051 MiB/s
Message latency: 3.255 mcs
Message throughput: 307154 msg/s
```

### HTTPS Trace server

* [cppserver-performance-https_trace_server](https://github.com/chronoxor/CppServer/blob/master/performance/https_trace_server.cpp)
* [cppserver-performance-https_trace_client](https://github.com/chronoxor/CppServer/blob/master/performance/https_trace_client.cpp) --clients 1 --threads 1

```
Server address: 127.0.0.1
Server port: 443
Working threads: 1
Working clients: 1
Working messages: 1
Seconds to benchmarking: 10

Errors: 0

Total time: 10.003 s
Total data: 37.475 MiB
Total messages: 370602
Data throughput: 3.763 MiB/s
Message latency: 26.992 mcs
Message throughput: 37047 msg/s
```

* [cppserver-performance-https_trace_server](https://github.com/chronoxor/CppServer/blob/master/performance/https_trace_server.cpp)
* [cppserver-performance-https_trace_client](https://github.com/chronoxor/CppServer/blob/master/performance/https_trace_client.cpp) --clients 100 --threads 4

```
Server address: 127.0.0.1
Server port: 443
Working threads: 4
Working clients: 100
Working messages: 1
Seconds to benchmarking: 10

Errors: 0

Total time: 10.035 s
Total data: 204.531 MiB
Total messages: 2023152
Data throughput: 20.389 MiB/s
Message latency: 4.960 mcs
Message throughput: 201602 msg/s
```

# OpenSSL certificates
In order to create OpenSSL based server and client you should prepare a set of
SSL certificates.

## Production
Depending on your project, you may need to purchase a traditional SSL
certificate signed by a Certificate Authority. If you, for instance,
want some else's web browser to talk to your WebSocket project, you'll
need a traditional SSL certificate.

## Development
The commands below entered in the order they are listed will generate a
self-signed certificate for development or testing purposes.

## Certificate Authority

* Create CA private key
```shell
openssl genrsa -passout pass:qwerty -out ca-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in ca-secret.key -out ca.key
```

* Create CA self-signed certificate
```shell
openssl req -new -x509 -days 3650 -subj '/C=BY/ST=Belarus/L=Minsk/O=Example root CA/OU=Example CA unit/CN=example.com' -key ca.key -out ca.crt
```

* Convert CA self-signed certificate to PFX
```shell
openssl pkcs12 -export -passout pass:qwerty -inkey ca.key -in ca.crt -out ca.pfx
```

* Convert CA self-signed certificate to PEM
```shell
openssl pkcs12 -passin pass:qwerty -passout pass:qwerty -in ca.pfx -out ca.pem
```

## SSL Server certificate

* Create private key for the server
```shell
openssl genrsa -passout pass:qwerty -out server-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in server-secret.key -out server.key
```

* Create CSR for the server
```shell
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example server/OU=Example server unit/CN=server.example.com' -key server.key -out server.csr
```

* Create certificate for the server
```shell
openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt
```

* Convert the server certificate to PFX
```shell
openssl pkcs12 -export -passout pass:qwerty -inkey server.key -in server.crt -out server.pfx
```

* Convert the server certificate to PEM
```shell
openssl pkcs12 -passin pass:qwerty -passout pass:qwerty -in server.pfx -out server.pem
```

## SSL Client certificate

* Create private key for the client
```shell
openssl genrsa -passout pass:qwerty -out client-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in client-secret.key -out client.key
```

* Create CSR for the client
```shell
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example client/OU=Example client unit/CN=client.example.com' -key client.key -out client.csr
```

* Create the client certificate
```shell
openssl x509 -req -days 3650 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt
```

* Convert the client certificate to PFX
```shell
openssl pkcs12 -export -passout pass:qwerty -inkey client.key -in client.crt -out client.pfx
```

* Convert the client certificate to PEM
```shell
openssl pkcs12 -passin pass:qwerty -passout pass:qwerty -in client.pfx -out client.pem
```

## Diffie-Hellman key exchange

* Create DH parameters
```shell
openssl dhparam -out dh4096.pem 4096
```
