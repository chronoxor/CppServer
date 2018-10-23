# CppServer

[![Linux build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppServer)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppServer)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppServer)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppServer)

Ultra fast and low latency asynchronous C++ Server & Client library with
support TCP, SSL, UDP protocols and 10K connection problem solution.

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
  * [Performance](#performance)
    * [Benchmark: Round-trip](#benchmark-round-trip)
      * [TCP echo server](#tcp-echo-server)
      * [SSL echo server](#ssl-echo-server)
      * [UDP echo server](#udp-echo-server)
    * [Benchmark: Multicast](#benchmark-multicast)
      * [TCP multicast server](#tcp-multicast-server)
      * [SSL multicast server](#ssl-multicast-server)
      * [UDP multicast server](#udp-multicast-server)
  * [OpenSSL certificates](#openssl-certificates)
    * [Certificate Authority](#certificate-authority)
    * [SSL Server certificate](#ssl-server-certificate)
    * [SSL Client certificate](#ssl-client-certificate)
    * [Diffie-Hellman key exchange](#diffie-hellman-key-exchange)

# Features
* Cross platform (Linux, OSX, Windows)
* [Asynchronous communication](https://think-async.com)
* Supported CPU scalability designs: IO service per thread, thread pool
* Supported transport protocols: [TCP](#example-tcp-chat-server), [SSL](#example-ssl-chat-server),
  [UDP](#example-udp-echo-server), [UDP multicast](#example-udp-multicast-server)

# Requirements
* Linux (gcc g++ cmake doxygen graphviz binutils-dev uuid-dev openssl)
* OSX (clang cmake doxygen graphviz openssl)
* Windows 7 / Windows 10
* [cmake](https://www.cmake.org)
* [git](https://git-scm.com)
* [gcc](https://gcc.gnu.org)

Optional:
* [clang](https://clang.llvm.org)
* [clion](https://www.jetbrains.com/clion)
* [MinGW](https://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

### Clone repository with submodules
```shell
git clone https://github.com/chronoxor/CppServer.git
cd CppServer
git submodule update --init --recursive --remote
```

### Linux
```shell
cd build
./unix.sh
```

### OSX
```shell
cd build
./unix.sh
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

class AsioService : public CppServer::Asio::Service
{
public:
    using CppServer::Asio::Service::Service;

protected:
    void onStarted() override
    {
        std::cout << "Asio service started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Asio service stopped!" << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Asio service caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Create a new Asio service
    auto service = std::make_shared<AsioService>();

    // Start the service
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

    // Stop the service
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
        Send(message);
    }
    void onDisconnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
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

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new TCP chat server
    auto server = std::make_shared<ChatServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

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

    // Stop the service
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

#include <iostream>

class ChatClient : public CppServer::Asio::TCPClient
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

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
        Connect();
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
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

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new TCP chat client
    auto client = std::make_shared<ChatClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
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
            client->Disconnect();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->Send(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop the service
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
    void onHandshaked() override
    {
        std::cout << "Chat SSL session with Id " << id() << " handshaked!" << std::endl;

        // Send invite message
        std::string message("Hello from SSL chat! Please send a message or '!' to disconnect the client!");
        Send(message.data(), message.size());
    }
    void onDisconnected() override
    {
        std::cout << "Chat SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message == "!")
            Disconnect();

        // Inform that we handled the whole buffer
        return size;
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
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(std::shared_ptr<CppServer::Asio::SSLServer> server, std::shared_ptr<asio::ssl::context> context) override
    {
        return std::make_shared<ChatSession>(server, context);
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
    int port = 3333;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "SSL server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL server context
    std::shared_ptr<asio::ssl::context> context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    context->set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    context->set_password_callback([](std::size_t max_length, asio::ssl::context::password_purpose purpose) -> std::string { return "qwerty"; });
    context->use_certificate_chain_file("../tools/certificates/server.pem");
    context->use_private_key_file("../tools/certificates/server.pem", asio::ssl::context::pem);
    context->use_tmp_dh_file("../tools/certificates/dh4096.pem");

    // Create a new SSL chat server
    auto server = std::make_shared<ChatServer>(service, context, CppServer::Asio::InternetProtocol::IPv4, port);

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

    // Stop the service
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

#include <iostream>

class ChatClient : public CppServer::Asio::SSLClient
{
public:
    using CppServer::Asio::SSLClient::SSLClient;

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
        Connect();
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat SSL client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // SSL server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // SSL server port
    int port = 3333;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "SSL server address: " << address << std::endl;
    std::cout << "SSL server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create and prepare a new SSL client context
    std::shared_ptr<asio::ssl::context> context = std::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);
    context->set_verify_mode(asio::ssl::verify_peer);
    context->load_verify_file("../tools/certificates/ca.pem");

    // Create a new SSL chat client
    auto client = std::make_shared<ChatClient>(service, context, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
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
            client->Disconnect();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the chat server
        client->Send(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop the service
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
    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::string message((const char*)buffer, size);
        std::cout << "Incoming: " << message << std::endl;

        // Echo the message back to the sender
        Send(endpoint, message);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Echo UDP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP server port
    int port = 2222;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "UDP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP echo server
    auto server = std::make_shared<EchoServer>(service, CppServer::Asio::InternetProtocol::IPv4, port);

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

    // Stop the service
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

#include <iostream>

class EchoClient : public CppServer::Asio::UDPClient
{
public:
    using CppServer::Asio::UDPClient::UDPClient;

protected:
    void onConnected() override
    {
        std::cout << "Echo UDP client connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Echo UDP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Echo UDP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // UDP server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // UDP server port
    int port = 2222;
    if (argc > 2)
        port = std::atoi(argv[2]);

    std::cout << "UDP server address: " << address << std::endl;
    std::cout << "UDP server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP echo client
    auto client = std::make_shared<EchoClient>(service, address, port);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop or '!' to disconnect the client..." << std::endl;

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
            client->Disconnect();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Send the entered text to the echo server
        client->Send(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop the service
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
    int multicast_port = 2223;
    if (argc > 2)
        multicast_port = std::atoi(argv[2]);

    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast server
    auto server = std::make_shared<MulticastServer>(service, CppServer::Asio::InternetProtocol::IPv4, 0);

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
        server->Multicast(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the service
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

#include <iostream>

class MulticastClient : public CppServer::Asio::UDPClient
{
public:
    std::string multicast;

public:
    using CppServer::Asio::UDPClient::UDPClient;

protected:
    void onConnected() override
    {
        std::cout << "Multicast UDP client connected a new session with Id " << id() << std::endl;

        // Join UDP multicast group
        JoinMulticastGroup(multicast);
    }
    void onDisconnected() override
    {
        std::cout << "Multicast UDP client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        std::cout << "Incoming: " << std::string((const char*)buffer, size) << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Multicast UDP client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
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
    int multicast_port = 2223;
    if (argc > 3)
        multicast_port = std::atoi(argv[3]);

    std::cout << "UDP listen address: " << listen_address << std::endl;
    std::cout << "UDP multicast address: " << multicast_address << std::endl;
    std::cout << "UDP multicast port: " << multicast_port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new UDP multicast client
    auto client = std::make_shared<MulticastClient>(service, listen_address, multicast_port, true);
    client->multicast = multicast_address;

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop or '!' to disconnect the client..." << std::endl;

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
            client->Disconnect();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop the service
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

## Benchmark: Round-trip

![Round-trip](https://github.com/chronoxor/CppServer/raw/master/images/round-trip.png)

This scenario sends lots of messages from several clients to a server.
The server responses to each message and resend the similar response to
the client. The benchmark measures total Round-trip time to send all
messages and receive all responses, messages & data throughput, count
of errors.

### TCP echo server

* [cppserver-performance-tcp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_server.cpp)
* [cppserver-performance-tcp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_client.cpp) -c 1 -m 1000000 -t 1

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 3.047 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 10.500 MiB/s
Message latency: 3.047 mcs
Message throughput: 328126 msg/s
```

* [cppserver-performance-tcp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_server.cpp)
* [cppserver-performance-tcp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_client.cpp) -c 100 -m 1000000 -t 4

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 4
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 1.186 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 26.972 MiB/s
Message latency: 1.186 mcs
Message throughput: 842887 msg/s
```

### SSL echo server

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) -c 1 -m 1000000 -t 1

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 7.917 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 4.042 MiB/s
Messages latency: 7.917 mcs
Messages throughput: 126309 msg/s
```

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) -c 100 -m 1000000 -t 4

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 4
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 4.741 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 6.749 MiB/s
Message latency: 4.741 mcs
Message throughput: 210912 msg/s
```

### UDP echo server

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) -c 1 -m 1000000 -t 1

```
Server address: 127.0.0.1
Server port: 3333
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 15.037 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 2.128 MiB/s
Messages latency: 15.037 mcs
Messages throughput: 66502 msg/s
```

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) -c 100 -m 1000000 -t 4

```
Server address: 127.0.0.1
Server port: 3333
Working threads: 4
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 4.499 s
Total bytes: 32000000
Total messages: 1000000
Data throughput: 7.112 MiB/s
Message latency: 4.499 mcs
Message throughput: 222236 msg/s
```

## Benchmark: Multicast

![Multicast](https://github.com/chronoxor/CppServer/raw/master/images/multicast.png)

In this scenario server multicasts messages to all connected clients.
The benchmark counts total messages received by all clients for all
the working time and measures messages & data throughput, count
of errors.

### TCP multicast server

* [cppserver-performance-tcp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_server.cpp)
* [cppserver-performance-tcp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_client.cpp) -c 1 -t 1

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 1
Working clients: 1
Message size: 32

Errors: 0

Multicast time: 10.007 s
Total data: 465.128 MiB
Total messages: 15241217
Data throughput: 46.491 MiB/s
Message latency: 656 ns
Message throughput: 1523045 msg/s
```

* [cppserver-performance-tcp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_server.cpp)
* [cppserver-performance-tcp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_multicast_client.cpp) -c 100 -t 4

```
Server address: 127.0.0.1
Server port: 1111
Working threads: 4
Working clients: 100
Message size: 32

Errors: 0

Multicast time: 10.012 s
Total data: 3.373 GiB
Total messages: 112902830
Data throughput: 344.127 MiB/s
Message latency: 88 ns
Message throughput: 11276267 msg/s
```

### SSL multicast server

* [cppserver-performance-ssl_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_server.cpp)
* [cppserver-performance-ssl_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_client.cpp) -c 1 -t 1

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 1
Working clients: 1
Message size: 32

Errors: 0

Multicast time: 10.008 s
Total data: 125.439 MiB
Total messages: 4110078
Data throughput: 12.545 MiB/s
Message latency: 2.435 mcs
Message throughput: 410666 msg/s
```

* [cppserver-performance-ssl_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_server.cpp)
* [cppserver-performance-ssl_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_multicast_client.cpp) -c 100 -t 4

```
Server address: 127.0.0.1
Server port: 2222
Working threads: 4
Working clients: 100
Message size: 32

Errors: 0

Multicast time: 10.013 s
Total data: 3.010 GiB
Total messages: 101017887
Data throughput: 307.878 MiB/s
Message latency: 99 ns
Message throughput: 10087872 msg/s
```

### UDP multicast server

* [cppserver-performance-udp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_server.cpp)
* [cppserver-performance-udp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_client.cpp) -c 1 -t 1

```
Server address: 239.255.0.1
Server port: 3333
Working threads: 1
Working clients: 1
Message size: 32

Errors: 0

Multicast time: 10.007 s
Total data: 19.516 MiB
Total messages: 639123
Data throughput: 1.971 MiB/s
Message latency: 15.657 mcs
Message throughput: 63867 msg/s
```

* [cppserver-performance-udp_multicast_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_server.cpp)
* [cppserver-performance-udp_multicast_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_multicast_client.cpp) -c 100 -t 4

```
Server address: 239.255.0.1
Server port: 3333
Working threads: 4
Working clients: 100
Message size: 32

Errors: 0

Multicast time: 10.012 s
Total data: 76.771 MiB
Total messages: 2515041
Data throughput: 7.681 MiB/s
Message latency: 3.981 mcs
Message throughput: 251184 msg/s
```

# OpenSSL certificates
In order to create OpenSSL based server and client you should prepare a set of
SSL certificates. Here comes several steps to get a self-signed set of SSL
certificates for testing purposes:

## Certificate Authority

* Create CA private key
```shell
openssl genrsa -des3 -passout pass:qwerty -out ca-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in ca-secret.key -out ca.key
```

* Create CA self-signed certificate
```shell
openssl req -new -x509 -days 3650 -subj '/C=BY/ST=Belarus/L=Minsk/O=Example root CA/OU=Example CA unit/CN=example.com' -key ca.key -out ca.crt -config openssl.cfg
```

* Convert CA self-signed certificate to PKCS
```shell
openssl pkcs12 -clcerts -export -passout pass:qwerty -in ca.crt -inkey ca.key -out ca.p12
```

* Convert CA self-signed certificate to PEM
```shell
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in ca.p12 -out ca.pem
```

## SSL Server certificate

* Create private key for the server
```shell
openssl genrsa -des3 -passout pass:qwerty -out server-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in server-secret.key -out server.key
```

* Create CSR for the server
```shell
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example server/OU=Example server unit/CN=server.example.com' -key server.key -out server.csr -config openssl.cfg
```

* Create certificate for the server
```shell
openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt
```

* Convert the server certificate to PKCS
```shell
openssl pkcs12 -clcerts -export -passout pass:qwerty -in server.crt -inkey server.key -out server.p12
```

* Convert the server certificate to PEM
```shell
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in server.p12 -out server.pem
```

## SSL Client certificate

* Create private key for the client
```shell
openssl genrsa -des3 -passout pass:qwerty -out client-secret.key 4096
```

* Remove passphrase
```shell
openssl rsa -passin pass:qwerty -in client-secret.key -out client.key
```

* Create CSR for the client
```shell
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example client/OU=Example client unit/CN=client.example.com' -key client.key -out client.csr -config openssl.cfg
```

* Create the client certificate
```shell
openssl x509 -req -days 3650 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt
```

* Convert the client certificate to PKCS
```shell
openssl pkcs12 -clcerts -export -passout pass:qwerty -in client.crt -inkey client.key -out client.p12
```

* Convert the client certificate to PEM
```shell
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in client.p12 -out client.pem
```

## Diffie-Hellman key exchange

* Create DH parameters
```shell
openssl dhparam -out dh4096.pem 4096
```
