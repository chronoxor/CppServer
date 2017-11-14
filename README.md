# CppServer

[![Linux build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppServer)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppServer/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppServer)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppServer)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppServer/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppServer)

C++ Server Library provides functionality to create different kind of
client/server solutions.

[CppServer API reference](http://chronoxor.github.io/CppServer/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
    * [Clone repository with submodules](#clone-repository-with-submodules)
    * [Linux](#linux)
    * [OSX](#osx)
    * [Windows (MinGW)](#windows-mingw)
    * [Windows (MinGW with MSYS)](#windows-mingw-with-msys)
    * [Windows (Visual Studio)](#windows-visual-studio)
  * [Asio](#asio)
    * [Asio service](#asio-service)
    * [Example: TCP chat server](#example-tcp-chat-server)
    * [Example: TCP chat client](#example-tcp-chat-client)
    * [Example: SSL chat server](#example-ssl-chat-server)
    * [Example: SSL chat client](#example-ssl-chat-client)
    * [Example: UDP echo server](#example-udp-echo-server)
    * [Example: UDP echo client](#example-udp-echo-client)
    * [Example: UDP multicast server](#example-udp-multicast-server)
    * [Example: UDP multicast client](#example-udp-multicast-client)
    * [Example: WebSocket chat server](#example-websocket-chat-server)
    * [Example: WebSocket chat client](#example-websocket-chat-client)
    * [Example: WebSocket SSL chat server](#example-websocket-ssl-chat-server)
    * [Example: WebSocket SSL chat client](#example-websocket-ssl-chat-client)
    * [Example: HTTP Web server](#example-http-web-server)
    * [Example: HTTPS Web server](#example-https-web-server)
    * [Example: HTTP/HTTPS Web synchronous client](#example-httphttps-web-synchronous-client)
    * [Example: HTTP/HTTPS Web asynchronous client](#example-httphttps-web-asynchronous-client)
  * [Nanomsg](#nanomsg)
    * [Example: Pair protocol](#example-pair-protocol)
    * [Example: Request/reply protocol](#example-requestreply-protocol)
    * [Example: Push/pull protocol](#example-pushpull-protocol)
    * [Example: Publish/subscribe protocol](#example-publishsubscribe-protocol)
    * [Example: Survey protocol](#example-survey-protocol)
    * [Example: Bus protocol](#example-bus-protocol)
  * [Performance](#performance)
    * [Benchmark: Round-Trip Time](#benchmark-round-trip-time)
      * [TCP echo server](#tcp-echo-server)
      * [SSL echo server](#ssl-echo-server)
      * [UDP echo server](#udp-echo-server)
      * [WebSocket echo server](#websocket-echo-server)
      * [WebSocket SSL echo server](#websocket-ssl-echo-server)
      * [HTTP echo server](#http-echo-server)
      * [HTTPS echo server](#http-echo-server)
      * [Nanomsg pair protocol](#nanomsg-pair-protocol)
      * [Nanomsg request/reply protocol](#nanomsg-requestreply-protocol)
  * [OpenSSL certificates](#openssl-certificates)
    * [Certificate Authority](#certificate-authority)
    * [SSL Server certificate](#ssl-server-certificate)
    * [SSL Client certificate](#ssl-client-certificate)
    * [Diffie-Hellman key exchange](#diffie-hellman-key-exchange)

# Features
* Cross platform (Linux, OSX, Windows)
* [Asynchronous communication](http://think-async.com)
* Supported transport protocols: [TCP](#example-tcp-chat-server), [SSL](#example-ssl-chat-server),
  [UDP](#example-udp-echo-server), [UDP multicast](#example-udp-multicast-server),
  [HTTP](#example-http-web-server), [HTTPS](#example-https-web-server),
  [WebSocket](#example-websocket-chat-server), [WebSocket SSL](#example-websocket-ssl-chat-server)

# Requirements
* Linux
* OSX
* Windows 7 / Windows 10
* [CMake](http://www.cmake.org)
* [GIT](https://git-scm.com)
* [GCC](https://gcc.gnu.org)

Optional:
* [Clang](http://clang.llvm.org)
* [Clion](https://www.jetbrains.com/clion)
* [MinGW](http://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

## Clone repository with submodules
```
git clone https://github.com/chronoxor/CppServer.git CppServer
cd CppServer
git submodule update --init --recursive --remote
```

## Linux
```
cd build
./unix.sh
```

## OSX
```
cd build
./unix.sh
```

## Windows (MinGW)
```
cd build
mingw.bat
```

## Windows (Visual Studio)
```
cd build
vs.bat
```

# Asio

## Asio service
Asio service is used to host all clients/servers based on [Asio C++ library](http://think-async.com).
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
```C++
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

## Example: TCP chat server
Here comes the example of the TCP chat server. It handles multiple TCP client
sessions and multicast received message from any session to all ones. Also it
is possible to send admin message directly from the server.

```C++
#include "server/asio/tcp_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::TCPServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::TCPServer<ChatServer, ChatSession>::TCPServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::TCPSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::TCPSession<ChatServer, ChatSession>::TCPSession;

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

```C++
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

```C++
#include "server/asio/ssl_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::SSLServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::SSLServer<ChatServer, ChatSession>::SSLServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat TCP server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::SSLSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::SSLSession<ChatServer, ChatSession>::SSLSession;

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

```C++
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

```C++
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

```C++
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

```C++
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

```C++
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

## Example: WebSocket chat server
Here comes the example of the WebSocket chat server. It handles multiple
WebSocket client sessions and multicast received message from any session
to all ones. Also it is possible to send admin message directly from the
server.

WebSocket server is based on [WebSocket++ library](https://www.zaphoyd.com/websocketpp)

```C++
#include "server/asio/websocket_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::WebSocketServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketServer<ChatServer, ChatSession>::WebSocketServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::WebSocketSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSession<ChatServer, ChatSession>::WebSocketSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocketCP session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket chat! Please send a message or '!' to disconnect the client!");
        Send(message);
    }
    void onDisconnected() override
    {
        std::cout << "Chat TCP session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(const CppServer::Asio::WebSocketMessage& message) override
    {
        std::cout << "Incoming: " << message->get_raw_payload() << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message->get_payload() == "!")
            Disconnect();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket server port
    int port = 4444;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "WebSocket server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new WebSocket chat server
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

## Example: WebSocket chat client
Here comes the example of the WebSocket chat client. It connects to the
WebSocket chat server and allows to send message to it and receive new
messages.

WebSocket client is based on [WebSocket++ library](https://www.zaphoyd.com/websocketpp)

```C++
#include "server/asio/websocket_client.h"
#include "threads/thread.h"

#include <iostream>

class ChatClient : public CppServer::Asio::WebSocketClient
{
public:
    using CppServer::Asio::WebSocketClient::WebSocketClient;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocket client connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Chat WebSocket client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(const CppServer::Asio::WebSocketMessage& message) override
    {
        std::cout << "Incoming: " << message->get_raw_payload() << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // WebSocket server port
    int port = 4444;
    if (argc > 2)
        port = std::atoi(argv[2]);

    // WebSocket server uri
    std::string uri = "ws://" + address + ":" + std::to_string(port);

    std::cout << "WebSocket server address: " << address << std::endl;
    std::cout << "WebSocket server port: " << port << std::endl;
    std::cout << "WebSocket server uri: " << uri << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new WebSocket chat client
    auto client = std::make_shared<ChatClient>(service, uri);

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

## Example: WebSocket SSL chat server
Here comes the example of the WebSocket chat server. It handles multiple
WebSocket clients sessions and multicast received message from any session
to all ones. Also it is possible to send admin message directly from the
server.

This example is very similar to the simple WebSocket one except the code that
prepares SSL context.

WebSocket server is based on [WebSocket++ library](https://www.zaphoyd.com/websocketpp)

```C++
#include "server/asio/websocket_ssl_server.h"
#include "threads/thread.h"

#include <iostream>

class ChatSession;

class ChatServer : public CppServer::Asio::WebSocketSSLServer<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSSLServer<ChatServer, ChatSession>::WebSocketSSLServer;

protected:
    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket SSL server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class ChatSession : public CppServer::Asio::WebSocketSSLSession<ChatServer, ChatSession>
{
public:
    using CppServer::Asio::WebSocketSSLSession<ChatServer, ChatSession>::WebSocketSSLSession;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocket SSL session with Id " << id() << " connected!" << std::endl;

        // Send invite message
        std::string message("Hello from WebSocket SSL chat! Please send a message or '!' to disconnect the client!");
        Send(message);
    }
    void onDisconnected() override
    {
        std::cout << "Chat WebSocket SSL session with Id " << id() << " disconnected!" << std::endl;
    }

    void onReceived(const CppServer::Asio::WebSocketSSLMessage& message) override
    {
        std::cout << "Incoming: " << message->get_raw_payload() << std::endl;

        // Multicast message to all connected sessions
        server()->Multicast(message);

        // If the buffer starts with '!' the disconnect the current session
        if (message->get_payload() == "!")
            Disconnect();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket SSL session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket SSL server port
    int port = 5555;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "WebSocket server port: " << port << std::endl;

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

    // Create a new WebSocket SSL chat server
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

## Example: WebSocket SSL chat client
Here comes the example of the WebSocket chat client. It connects to the
WebSocket chat server and allows to send message to it and receive new
messages.

This example is very similar to the simple WebSocket one except the code that
prepares SSL context.

WebSocket client is based on [WebSocket++ library](https://www.zaphoyd.com/websocketpp)

```C++
#include "server/asio/websocket_ssl_client.h"
#include "threads/thread.h"

#include <iostream>

class ChatClient : public CppServer::Asio::WebSocketSSLClient
{
public:
    using CppServer::Asio::WebSocketSSLClient::WebSocketSSLClient;

protected:
    void onConnected() override
    {
        std::cout << "Chat WebSocket SSL client connected a new session with Id " << id() << std::endl;
    }
    void onDisconnected() override
    {
        std::cout << "Chat WebSocket SSL client disconnected a session with Id " << id() << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(const CppServer::Asio::WebSocketSSLMessage& message) override
    {
        std::cout << "Incoming: " << message->get_raw_payload() << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Chat WebSocket SSL client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // WebSocket SSL server address
    std::string address = "127.0.0.1";
    if (argc > 1)
        address = argv[1];

    // WebSocket SSL server port
    int port = 5555;
    if (argc > 2)
        port = std::atoi(argv[2]);

    // WebSocket SSL server uri
    std::string uri = "wss://" + address + ":" + std::to_string(port);

    std::cout << "WebSocket SSL server address: " << address << std::endl;
    std::cout << "WebSocket SSL server port: " << port << std::endl;
    std::cout << "WebSocket SSL server uri: " << uri << std::endl;

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

    // Create a new WebSocket SSL chat client
    auto client = std::make_shared<ChatClient>(service, context, uri);

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
    std::cout << "Done!" << std::endl;..

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTP Web server
Here comes the example of the HTTP Web server. It handles RESTful Web requests
to create, get, update and delete key/value storage data.

HTTP Web server is based on [Restbed library](https://github.com/Corvusoft/restbed)

```C++
#include "server/asio/web_server.h"

#include <iostream>
#include <memory>
#include <map>

class HttpServer : public CppServer::Asio::WebServer
{
public:
    explicit HttpServer(std::shared_ptr<CppServer::Asio::Service> service, int port)
        : CppServer::Asio::WebServer(service, port, false)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage/{key: .*}");
        resource->set_method_handler("POST", RestStoragePost);
        resource->set_method_handler("GET", RestStorageGet);
        resource->set_method_handler("PUT", RestStoragePut);
        resource->set_method_handler("DELETE", RestStorageDelete);

        // Publish the resource
        server()->publish(resource);
    }

private:
    static std::map<std::string, std::string> _storage;

    static void RestStoragePost(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "POST /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageGet(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "GET /storage/" << key << " => " << data << std::endl;

        session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
    }

    static void RestStoragePut(const std::shared_ptr<restbed::Session>& session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageDelete(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "DELETE /storage/" << key << " => " << data << std::endl;

        _storage[key] = "";

        session->close(restbed::OK);
    }
};

std::map<std::string, std::string> HttpServer::_storage;

int main(int argc, char** argv)
{
    // HTTP Web server port
    int port = 8000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "HTTP Web server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP Web server
    auto server = std::make_shared<HttpServer>(service, port);

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

## Example: HTTPS Web server
Here comes the example of the HTTPS Web server. It handles RESTful Web requests
to create, get, update and delete key/value storage data.

This example is very similar to the simple HTTP Web server one except the SSL
secured transport is used.

HTTPS Web server is based on [Restbed library](https://github.com/Corvusoft/restbed)

```C++
#include "server/asio/web_server.h"

#include <iostream>
#include <memory>
#include <map>

class HttpsServer : public CppServer::Asio::WebServer
{
public:
    explicit HttpsServer(std::shared_ptr<CppServer::Asio::Service> service, int port)
        : CppServer::Asio::WebServer(service, port, true)
    {
        // Create a resource
        auto resource = std::make_shared<restbed::Resource>();
        resource->set_path("/storage/{key: .*}");
        resource->set_method_handler("POST", RestStoragePost);
        resource->set_method_handler("GET", RestStorageGet);
        resource->set_method_handler("PUT", RestStoragePut);
        resource->set_method_handler("DELETE", RestStorageDelete);

        // Publish the resource
        server()->publish(resource);

        // Prepare SSL settings
        ssl_settings()->set_http_disabled(true);
        ssl_settings()->set_default_workarounds_enabled(true);
        ssl_settings()->set_sslv2_enabled(false);
        ssl_settings()->set_single_diffie_hellman_use_enabled(true);
        ssl_settings()->set_passphrase("qwerty");
        ssl_settings()->set_certificate_chain(restbed::Uri("file://../tools/certificates/server.pem"));
        ssl_settings()->set_private_key(restbed::Uri("file://../tools/certificates/server.pem"));
        ssl_settings()->set_temporary_diffie_hellman(restbed::Uri("file://../tools/certificates/dh4096.pem"));
    }

private:
    static std::map<std::string, std::string> _storage;

    static void RestStoragePost(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "POST /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageGet(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "GET /storage/" << key << " => " << data << std::endl;

        session->close(restbed::OK, data, { { "Content-Length", std::to_string(data.size()) } });
    }

    static void RestStoragePut(const std::shared_ptr<restbed::Session>& session)
    {
        const auto request = session->get_request();
        size_t request_content_length = request->get_header("Content-Length", 0);
        session->fetch(request_content_length, [request](const std::shared_ptr<restbed::Session> session, const restbed::Bytes & body)
        {
            std::string key = request->get_path_parameter("key");
            std::string data = std::string((char*)body.data(), body.size());

            std::cout << "PUT /storage/" << key << " => " << data << std::endl;

            _storage[key] = data;

            session->close(restbed::OK);
        });
    }

    static void RestStorageDelete(const std::shared_ptr<restbed::Session>& session)
    {
        auto request = session->get_request();
        std::string key = request->get_path_parameter("key");
        std::string data = _storage[key];

        std::cout << "DELETE /storage/" << key << " => " << data << std::endl;

        _storage[key] = "";

        session->close(restbed::OK);
    }
};

std::map<std::string, std::string> HttpsServer::_storage;

int main(int argc, char** argv)
{
    // HTTPS Web server port
    int port = 9000;
    if (argc > 1)
        port = std::atoi(argv[1]);

    std::cout << "HTTPS Web server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTPS Web server
    auto server = std::make_shared<HttpsServer>(service, port);

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

## Example: HTTP/HTTPS Web synchronous client
Here comes the example of the HTTP Web synchronous client. It send GET Web
request and receive response from a Web server in synchronous mode.

HTTP/HTTPS Web synchronous client is based on [Restbed library](https://github.com/Corvusoft/restbed)

```C++
#include "server/asio/web_client.h"

#include <iostream>
#include <memory>

void Show(const std::shared_ptr<restbed::Response>& response)
{
    std::cout << "*** Response header ***" << std::endl;
    std::cout << "Status Code:    " << response->get_status_code() << std::endl;
    std::cout << "Status Message: " << response->get_status_message().data() << std::endl;
    std::cout << "HTTP Version:   " << response->get_version() << std::endl;
    std::cout << "HTTP Protocol:  " << response->get_protocol().data() << std::endl;
    for (auto& header : response->get_headers())
        std::cout << "Header ['" << header.first.data() << "'] = '" << header.second.data() << "'" << std::endl;

    std::cout << "*** Response body ***" << std::endl;
    auto length = response->get_header("Content-Length", 0);
    auto content = CppServer::Asio::WebClient::Fetch(response, length);
    std::cout.write((char*)content.data(), content.size());
    std::cout << std::endl << "*** Response end ***" << std::endl;
}

int main(int argc, char** argv)
{
    // HTTP/HTTPS Web server address
    std::string address = "https://www.google.com";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTP/HTTPS Web server address: " << address << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP/HTTPS Web client
    auto client = std::make_shared<CppServer::Asio::WebClient>(service, false);

    // Create and fill Web request
    auto request = std::make_shared<restbed::Request>(restbed::Uri(address));
    request->set_header("Accept", "*/*");

    try
    {
        // Send synchronous Web request to the server
        auto response = client->Send(request);

        // Show the Web response
        Show(response);
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: HTTP/HTTPS Web asynchronous client
Here comes the example of the HTTP Web synchronous client. It send GET Web
request and receive response from a Web server in asynchronous mode.

HTTP/HTTPS Web asynchronous client is based on [Restbed library](https://github.com/Corvusoft/restbed)

```C++
#include "server/asio/web_client.h"

#include <iostream>
#include <memory>

void Show(const std::shared_ptr<restbed::Response>& response)
{
    std::cout << "*** Response header ***" << std::endl;
    std::cout << "Status Code:    " << response->get_status_code() << std::endl;
    std::cout << "Status Message: " << response->get_status_message().data() << std::endl;
    std::cout << "HTTP Version:   " << response->get_version() << std::endl;
    std::cout << "HTTP Protocol:  " << response->get_protocol().data() << std::endl;
    for (auto& header : response->get_headers())
        std::cout << "Header ['" << header.first.data() << "'] = '" << header.second.data() << "'" << std::endl;

    std::cout << "*** Response body ***" << std::endl;
    auto length = response->get_header("Content-Length", 0);
    auto content = CppServer::Asio::WebClient::Fetch(response, length);
    std::cout.write((char*)content.data(), content.size());
    std::cout << std::endl << "*** Response end ***" << std::endl;
}

int main(int argc, char** argv)
{
    // HTTP/HTTPS Web server address
    std::string address = "https://www.google.com";
    if (argc > 1)
        address = argv[1];

    std::cout << "HTTP/HTTPS Web server address: " << address << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<CppServer::Asio::Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new HTTP/HTTPS Web client
    auto client = std::make_shared<CppServer::Asio::WebClient>(service, false);

    // Create and fill Web request
    auto request = std::make_shared<restbed::Request>(restbed::Uri(address));
    request->set_header("Accept", "*/*");

    // Send synchronous Web request to the server
    auto response = client->SendAsync(request, [](const std::shared_ptr<restbed::Request>& request, const std::shared_ptr<restbed::Response>& response)
    {
        // Show the Web response
        Show(response);
    });

    std::cout << "Press Enter to stop the client..." << std::endl;
    std::string line;
    getline(std::cin, line);

    try
    {
        if (response.valid())
            response.get();
    }
    catch (std::exception& ex)
    {
        std::cerr << "Exception caught: " << ex.what() << std::endl;
    }

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

# Nanomsg
[Nanomsg](http://nanomsg.org) is a socket library that provides several common
communication patterns. It aims to make the networking layer fast, scalable, and
easy to use. Implemented in C, it works on a wide range of operating systems with
no further dependencies.

The communication patterns, also called "scalability protocols", are basic blocks
for building distributed systems. By combining them you can create a vast array
of distributed applications. The following scalability protocols are currently
available:

* **PAIR** - simple one-to-one communication
* **BUS** - simple many-to-many communication
* **REQREP** - allows to build clusters of stateless services to process user requests
* **PUBSUB** - distributes messages to large sets of interested subscribers
* **PIPELINE** - aggregates messages from multiple sources and load balances them among many destinations
* **SURVEY** - allows to query state of multiple applications in a single go

Scalability protocols are layered on top of the transport layer in the network
stack. At the moment, the nanomsg library supports the following transports
mechanisms:

* **INPROC** - transport within a process (between threads, modules etc.)
* **IPC** - transport between processes on a single machine
* **TCP** - network transport via TCP

## Example: Pair protocol
Pair protocol is the simplest and least scalable scalability protocol. It allows
scaling by breaking the application in exactly two pieces. For example, if a
monolithic application handles both accounting and agenda of HR department, it
can be split into two applications (accounting vs. HR) that are run on two separate
servers. These applications can then communicate via PAIR sockets.

The downside of this protocol is that its scaling properties are very limited.
Splitting the application into two pieces allows to scale the two servers.
To add the third server to the cluster, the application has to be split once more,
say by separating HR functionality into hiring module and salary computation module.
Whenever possible, try to use one of the more scalable protocols instead.

Socket Types
* **NN_PAIR** - Socket for communication with exactly one peer. Each party can
  send messages at any time. If the peer is not available or send buffer is full
  subsequent calls to nn_send(3) will block until it's possible to send the
  message.

Socket Options
* No protocol-specific socket options are defined at the moment.

Here comes the example of the Nanomsg pair server:

```C++
#include "server/nanomsg/pair_server.h"

#include <iostream>
#include <memory>

class ExamplePairServer : public CppServer::Nanomsg::PairServer
{
public:
    using CppServer::Nanomsg::PairServer::PairServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg pair server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg pair server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;

        // Send the reversed message back to the client
        std::string result(message.string());
        Send(std::string(result.rbegin(), result.rend()));
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg pair server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg pair server address
    std::string address = "tcp://127.0.0.1:6667";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg pair server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg pair server
    auto server = std::make_shared<ExamplePairServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

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

        // Send the entered text to the client
        server->Send(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

Here comes the example of the Nanomsg pair client:

```C++
#include "server/nanomsg/pair_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExamplePairClient : public CppServer::Nanomsg::PairClient
{
public:
    using CppServer::Nanomsg::PairClient::PairClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg pair client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg pair client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg pair client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg pair server address
    std::string address = "tcp://127.0.0.1:6667";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg pair server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg pair client
    auto client = std::make_shared<ExamplePairClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

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

        // Send the entered text to the server
        client->Send(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: Request/reply protocol
This protocol is used to distribute the workload among multiple stateless workers.

Please note that request/reply applications should be stateless.

It's important to include all the information necessary to process the request in
the request itself, including information about the sender or the originator of
the request if this is necessary to respond to the request.

Sender information cannot be retrieved from the underlying socket connection since,
firstly, transports like IPC may not have a firm notion of a message origin. Secondly,
transports that have some notion may not have a reliable one - a TCP disconnect may
mean a new sender, or it may mean a temporary loss in network connectivity.

For this reason, sender information must be included by the application if required.
Allocating 6 randomly-generated bytes in the message for the lifetime of the connection
is sufficient for most purposes. For longer-lived applications, an UUID is more suitable.

Socket Types
* **NN_REQ** - Used to implement the client application that sends requests and
  receives replies.
* **NN_REP** - Used to implement the stateless worker that receives requests and
  sends replies.

Socket Options
* **NN_REQ_RESEND_IVL** - This option is defined on the full REQ socket. If reply
  is not received in specified amount of milliseconds, the request will be
  automatically resent. The type of this option is int. Default value is 60000
  (1 minute).

Here comes the example of the Nanomsg request/reply server:

```C++
#include "server/nanomsg/request_server.h"

#include <iostream>
#include <memory>

class ExampleRequestServer : public CppServer::Nanomsg::RequestServer
{
public:
    using CppServer::Nanomsg::RequestServer::RequestServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg request server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg request server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;

        // Send the reversed message back to the client
        std::string result(message.string());
        Send(std::string(result.rbegin(), result.rend()));
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg request server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg request server address
    std::string address = "tcp://127.0.0.1:6668";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg request server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg request server
    auto server = std::make_shared<ExampleRequestServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

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

    return 0;
}
```

Here comes the example of the Nanomsg request/reply client:

```C++
#include "server/nanomsg/request_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExampleRequestClient : public CppServer::Nanomsg::RequestClient
{
public:
    using CppServer::Nanomsg::RequestClient::RequestClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg request client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg request client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg request client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg reply server address
    std::string address = "tcp://127.0.0.1:6668";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg reply server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg request client
    auto client = std::make_shared<ExampleRequestClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

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

        // Request the entered text to the server
        CppServer::Nanomsg::Message message = client->Request(line);

        // Show the response message
        std::cout << "Response: " << message << std::endl;
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: Push/pull protocol
Fair queues messages from the previous processing step and load balances them
among instances of the next processing step.

Socket Types
* **NN_PUSH** - This socket is used to send messages to a cluster of load-balanced
  nodes. Receive operation is not implemented on this socket type.
* **NN_PULL** - This socket is used to receive a message from a cluster of nodes.
  Send operation is not implemented on this socket type.

Socket Options
* No protocol-specific socket options are defined at the moment.

Here comes the example of the Nanomsg push/pull server:

```C++
#include "server/nanomsg/push_server.h"

#include <iostream>
#include <memory>

class ExamplePushServer : public CppServer::Nanomsg::PushServer
{
public:
    using CppServer::Nanomsg::PushServer::PushServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg push server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg push server stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg push server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg push server address
    std::string address = "tcp://127.0.0.1:6666";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg push server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg push server
    auto server = std::make_shared<ExamplePushServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

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

    return 0;
}
```

Here comes the example of the Nanomsg push/pull client:

```C++
#include "server/nanomsg/push_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExamplePushClient : public CppServer::Nanomsg::PushClient
{
public:
    using CppServer::Nanomsg::PushClient::PushClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg push client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg push client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg push client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg push server address
    std::string address = "tcp://127.0.0.1:6666";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg push server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg push client
    auto client = std::make_shared<ExamplePushClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

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

        // Send the entered text to the server
        client->Send(line);
    }

    // Disconnect the client
    std::cout << "Client disconnecting...";
    client->Disconnect();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

## Example: Publish/subscribe protocol
Broadcasts messages to multiple destinations.

Messages are sent from **NN_PUB** sockets and will only be received by **NN_SUB**
sockets that have subscribed to the matching topic. Topic is an arbitrary sequence
of bytes at the beginning of the message body. The **NN_SUB** socket will determine
whether a message should be delivered to the user by comparing the subscribed
topics (using **NN_SUB_SUBSCRIBE** on a full SUB socket) to the bytes initial
bytes in the incoming message, up to the size of the topic.

```C++
nn_setsockopt (s, NN_SUB, NN_SUB_SUBSCRIBE, "Hello", 5);
```

Will match any message with intial 5 bytes being "Hello", for example, message
"Hello, World!" will match.

Topic with zero length matches any message.

If the socket is subscribed to multiple topics, message matching any of them
will be delivered to the user.

Since the filtering is performed on the Subscriber side, all the messages from
Publisher will be sent over the transport layer.

The entire message, including the topic, is delivered to the user.

Socket Types
* **NN_PUB** - This socket is used to distribute messages to multiple destinations.
  Receive operation is not defined.
* **NN_SUB** - Receives messages from the publisher. Only messages that the socket
  is subscribed to are received. When the socket is created there are no
  subscriptions and thus no messages will be received. Send operation is
  not defined on this socket.

Socket Options
* **NN_SUB_SUBSCRIBE** - Defined on full SUB socket. Subscribes for a particular
  topic. Type of the option is string. A single NN_SUB socket can handle multiple
  subscriptions.
* **NN_SUB_UNSUBSCRIBE** - Defined on full SUB socket. Unsubscribes from a particular
  topic. Type of the option is string.

Here comes the example of the Nanomsg publish/subscribe server:

```C++
#include "server/nanomsg/subscribe_server.h"

#include <iostream>
#include <memory>

class ExampleSubscribeServer : public CppServer::Nanomsg::SubscribeServer
{
public:
    using CppServer::Nanomsg::SubscribeServer::SubscribeServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg subscribe server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg subscribe server stopped!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg subscribe server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg subscribe server address
    std::string address = "tcp://127.0.0.1:6669";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg subscribe server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg subscribe server
    auto server = std::make_shared<ExampleSubscribeServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

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

        // Publish the entered text to all subscribed clients
        server->Send(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

Here comes the example of the Nanomsg publish/subscribe client:

```C++
#include "server/nanomsg/subscribe_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExampleSubscribeClient : public CppServer::Nanomsg::SubscribeClient
{
public:
    using CppServer::Nanomsg::SubscribeClient::SubscribeClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg subscribe client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg subscribe client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg subscribe client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg publisher server address
    std::string address = "tcp://127.0.0.1:6669";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg publisher server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg subscribe client
    auto client = std::make_shared<ExampleSubscribeClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

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

    return 0;
}
```

## Example: Survey protocol
Allows to broadcast a survey to multiple locations and gather the responses.

Socket Types
* **NN_SURVEYOR** - Used to send the survey. The survey is delivered to all the
  connected respondents. Once the query is sent, the socket can be used to
  receive the responses. When the survey deadline expires, receive will return
  ETIMEDOUT error.
* **NN_RESPONDENT** - Use to respond to the survey. Survey is received using receive
  function, response is sent using send function. This socket can be connected
  to at most one peer.

Socket Options
* **NN_SURVEYOR_DEADLINE** - Specifies how long to wait for responses to the survey.
  Once the deadline expires, receive function will return ETIMEDOUT error and all
  subsequent responses to the survey will be silently dropped. The deadline is
  measured in milliseconds. Option type is int. Default value is 1000 (1 second).

Here comes the example of the Nanomsg survey server:

```C++
#include "server/nanomsg/survey_server.h"

#include <iostream>
#include <memory>

class ExampleSurveyServer : public CppServer::Nanomsg::SurveyServer
{
public:
    using CppServer::Nanomsg::SurveyServer::SurveyServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg survey server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg survey server stopped!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg survey server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg survey server address
    std::string address = "tcp://127.0.0.1:6670";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg survey server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg survey server
    auto server = std::make_shared<ExampleSurveyServer>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

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

        // Survey the entered text to all respondent clients
        if (server->Send(line) == line.size())
        {
            // Start the survey
            std::cout << "Survey started! Question: " << line << std::endl;
            while (true)
            {
                CppServer::Nanomsg::Message message;

                // Receive survey responses from clients
                std::tuple<size_t, bool> result = server->ReceiveSurvey(message);

                // Show answers from respondents
                if (std::get<0>(result) > 0)
                    std::cout << "Answer: " << message << std::endl;

                // Finish the survey
                if (std::get<1>(result))
                {
                    std::cout << "Survey finished!" << std::endl;
                    break;
                }
            }
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
```

Here comes the example of the Nanomsg survey client:

```C++
#include "server/nanomsg/survey_client.h"
#include "threads/thread.h"

#include <iostream>
#include <memory>

class ExampleSurveyClient : public CppServer::Nanomsg::SurveyClient
{
public:
    using CppServer::Nanomsg::SurveyClient::SurveyClient;

protected:
    void onConnected() override
    {
        std::cout << "Nanomsg survey client connected" << std::endl;
    }

    void onDisconnected() override
    {
        std::cout << "Nanomsg survey client disconnected" << std::endl;

        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;

        // Send the reversed respond message back to the server
        std::string result(message.string());
        Send(std::string(result.rbegin(), result.rend()));
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg survey client caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg surveyor server address
    std::string address = "tcp://127.0.0.1:6670";
    if (argc > 1)
        address = argv[1];

    std::cout << "Nanomsg surveyor server address: " << address << std::endl;
    std::cout << "Press Enter to stop the client or '!' to reconnect the client..." << std::endl;

    // Create a new Nanomsg survey client
    auto client = std::make_shared<ExampleSurveyClient>(address);

    // Connect the client
    std::cout << "Client connecting...";
    client->Connect();
    std::cout << "Done!" << std::endl;

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

    return 0;
}
```

## Example: Bus protocol
Broadcasts messages from any node to all other nodes in the topology. The socket
should never receive messages that it sent itself.

This pattern scales only to local level (within a single machine or within a single
LAN). Trying to scale it further can result in overloading individual nodes with
messages.

**WARNING: For bus topology to function correctly, user is responsible for ensuring
that path from each node to any other node exists within the topology. Raw (AF_SP_RAW)
BUS socket never sends the message to the peer it was received from.**

Socket Types
* **NN_BUS** - Sent messages are distributed to all nodes in the topology.
  Incoming messages from all other nodes in the topology are fair-queued in
  the socket.

Socket Options
There are no options defined at the moment.

Here comes the example of the Nanomsg bus node:

```C++
#include "server/nanomsg/bus.h"

#include <iostream>
#include <memory>

class ExampleBus : public CppServer::Nanomsg::Bus
{
public:
    using CppServer::Nanomsg::Bus::Bus;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg bus node started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg bus node stopped!" << std::endl;
    }

    void onReceived(CppServer::Nanomsg::Message& message) override
    {
        std::cout << "Incoming: " << message << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg bus node caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: nanomsg_bus address [node1] [node2]..." << std::endl;
        return -1;
    }

    // Nanomsg bus node address
    std::string address = argv[1];

    std::cout << "Nanomsg bus node address: " << address << std::endl;
    std::cout << "Press Enter to stop the bus node or '!' to restart the bus node..." << std::endl;

    // Create a new Nanomsg bus node
    auto server = std::make_shared<ExampleBus>(address);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    // Link the bus node to another bus nodes
    for (int i = 2; i < argc; ++i)
    {
        std::string link_address = argv[i];
        server->Link(link_address);
        std::cout << "Nanomsg bus node linked: " << link_address << std::endl;
    }

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

        // Send the entered text to all connected bus nodes
        server->Send(line);
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
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

## Benchmark: Round-Trip Time

This scenario sends lots of messages from several clients to a server.
The server responses to each message and resend the similar response to
the client. The benchmark measures total Round-trip time to send all
messages and receive all responses, messages & bytes throughput, count
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

Round-trip time: 3.208 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 9974318 bytes per second
Message latency: 3.208 mcs
Message throughput: 311697 messages per second
```

* [cppserver-performance-tcp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_server.cpp)
* [cppserver-performance-tcp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/tcp_echo_client.cpp) -c 100 -m 1000000 -t 8
```
Server address: 127.0.0.1
Server port: 1111
Working threads: 8
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 1.625 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 19691928 bytes per second
Message latency: 1.625 mcs
Message throughput: 615372 messages per second
```

### SSL echo server

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) -c 1 -m 1000000 -t 1
```
Server address: 127.0.0.1
Server port: 3333
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 7.917 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 4041891 bytes per second
Messages latency: 7.917 mcs
Messages throughput: 126309 messages per second
```

* [cppserver-performance-ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_server.cpp)
* [cppserver-performance-ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/ssl_echo_client.cpp) -c 100 -m 1000000 -t 8
```
Server address: 127.0.0.1
Server port: 3333
Working threads: 8
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 9.679 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 3305832 bytes per second
Messages latency: 9.679 mcs
Messages throughput: 103307 messages per second
```

### UDP echo server

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) -c 1 -m 1000000 -t 1
```
Server address: 127.0.0.1
Server port: 2222
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 15.037 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 2128070 bytes per second
Messages latency: 15.037 mcs
Messages throughput: 66502 messages per second
```

* [cppserver-performance-udp_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_server.cpp)
* [cppserver-performance-udp_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/udp_echo_client.cpp) -c 100 -m 1000000 -t 8
```
Server address: 127.0.0.1
Server port: 2222
Working threads: 8
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 4.648 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 6884430 bytes per second
Messages latency: 4.648 mcs
Messages throughput: 215138 messages per second
```

### WebSocket echo server

* [cppserver-performance-websocket_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_echo_server.cpp)
* [cppserver-performance-websocket_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_echo_client.cpp) -c 1 -m 1000000 -t 1
```
Server address: 127.0.0.1
Server port: 4444
Server uri: ws://127.0.0.1:4444
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 35.323 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 905916 bytes per second
Messages latency: 35.323 mcs
Messages throughput: 28309 messages per second
```

* [cppserver-performance-websocket_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_echo_server.cpp)
* [cppserver-performance-websocket_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_echo_client.cpp) -c 100 -m 1000000 -t 8
```
Server address: 127.0.0.1
Server port: 4444
Server uri: ws://127.0.0.1:4444
Working threads: 8
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 10.117 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 3162890 bytes per second
Messages latency: 10.117 mcs
Messages throughput: 98840 messages per second
```

### WebSocket SSL echo server

* [cppserver-performance-websocket_ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_ssl_echo_server.cpp)
* [cppserver-performance-websocket_ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_ssl_echo_client.cpp) -c 1 -m 1000000 -t 1
```
Server address: 127.0.0.1
Server port: 5555
Server uri: wss://127.0.0.1:5555
Working threads: 1
Working clients: 1
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 1:10.267 m
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 455401 bytes per second
Messages latency: 70.267 mcs
Messages throughput: 14231 messages per second
```

* [cppserver-performance-websocket_ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_ssl_echo_server.cpp)
* [cppserver-performance-websocket_ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/websocket_ssl_echo_client.cpp) -c 100 -m 1000000 -t 8
```
Server address: 127.0.0.1
Server port: 5555
Server uri: wss://127.0.0.1:5555
Working threads: 8
Working clients: 100
Messages to send: 1000000
Message size: 32

Errors: 0

Round-trip time: 36.551 s
Total bytes: 32000000
Total messages: 1000000
Bytes throughput: 875468 bytes per second
Messages latency: 36.551 mcs
Messages throughput: 27358 messages per second
```

### HTTP echo server

* [cppserver-performance-web_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/web_echo_server.cpp)
* [cppserver-performance-web_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/web_echo_client.cpp) -c 1 -m 10000 -t 1
```
Server address: 127.0.0.1
Server port: 8000
Server uri: http://127.0.0.1:8000/storage
Working threads: 1
Working clients: 1
Messages to send: 10000
Message size: 32

Errors: 0

Round-trip time: 7.197 s
Total bytes: 320000
Total messages: 10000
Bytes throughput: 44462 bytes per second
Messages latency: 719.732 mcs
Messages throughput: 1389 messages per second
```

* [cppserver-performance-web_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/web_echo_server.cpp)
* [cppserver-performance-web_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/web_echo_client.cpp) -c 100 -m 10000 -t 8
```
Server address: 127.0.0.1
Server port: 8000
Server uri: http://127.0.0.1:8000/storage
Working threads: 8
Working clients: 100
Messages to send: 10000
Message size: 32

Errors: 0

Round-trip time: 2.780 s
Total bytes: 320000
Total messages: 10000
Bytes throughput: 115085 bytes per second
Messages latency: 278.027 mcs
Messages throughput: 3596 messages per second
```

### HTTPS echo server

* [cppserver-performance-web_ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/web_ssl_echo_server.cpp)
* [cppserver-performance-web_ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/web_ssl_echo_client.cpp) -c 1 -m 1000 -t 1
```
Server address: 127.0.0.1
Server port: 9000
Server uri: https://127.0.0.1:9000/storage
Working threads: 1
Working clients: 1
Messages to send: 1000
Message size: 32

Errors: 0

Round-trip time: 8.105 s
Total bytes: 32000
Total messages: 1000
Bytes throughput: 3947 bytes per second
Messages latency: 8.105 ms
Messages throughput: 123 messages per second
```

* [cppserver-performance-web_ssl_echo_server](https://github.com/chronoxor/CppServer/blob/master/performance/web_ssl_echo_server.cpp)
* [cppserver-performance-web_ssl_echo_client](https://github.com/chronoxor/CppServer/blob/master/performance/web_ssl_echo_client.cpp) -c 100 -m 1000 -t 8
```
Server address: 127.0.0.1
Server port: 9000
Server uri: https://127.0.0.1:9000/storage
Working threads: 8
Working clients: 100
Messages to send: 1000
Message size: 32

Errors: 0

Round-trip time: 6.823 s
Total bytes: 32000
Total messages: 1000
Bytes throughput: 4689 bytes per second
Messages latency: 6.823 ms
Messages throughput: 146 messages per second
```

### Nanomsg pair protocol

* [cppserver-performance-nanomsg_pair_server](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_pair_server.cpp)
* [cppserver-performance-nanomsg_pair_client](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_pair_client.cpp) -m 100000
```
Server address: tcp://127.0.0.1:6666
Messages to send: 100000
Message size: 32

Errors: 0

Round-trip time: 2.158 s
Total bytes: 3200000
Total messages: 100000
Bytes throughput: 1482697 bytes per second
Messages latency: 21.582 mcs
Messages throughput: 46334 messages per second
```

### Nanomsg request/reply protocol

* [cppserver-performance-nanomsg_request_server](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_request_server.cpp)
* [cppserver-performance-nanomsg_request_client](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_request_client.cpp) -c 1 -m 100000
```
Server address: tcp://127.0.0.1:6666
Working clients: 1
Messages to send: 100000
Message size: 32

Errors: 0

Round-trip time: 5.401 s
Total bytes: 3200000
Total messages: 100000
Bytes throughput: 592387 bytes per second
Messages latency: 54.018 mcs
Messages throughput: 18512 messages per second
```

* [cppserver-performance-nanomsg_request_server](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_request_server.cpp)
* [cppserver-performance-nanomsg_request_client](https://github.com/chronoxor/CppServer/blob/master/performance/nanomsg_request_client.cpp) -c 100 -m 100000
```
Server address: tcp://127.0.0.1:6666
Working clients: 100
Messages to send: 100000
Message size: 32

Errors: 0

Round-trip time: 6.159 s
Total bytes: 3200000
Total messages: 100000
Bytes throughput: 519542 bytes per second
Messages latency: 61.592 mcs
Messages throughput: 16235 messages per second
```

# OpenSSL certificates
In order to create OpenSSL based server and client you should prepare a set of
SSL certificates. Here comes several steps to get a self-signed set of SSL
certificates for testing purposes:

## Certificate Authority

* Create CA private key
```
openssl genrsa -des3 -passout pass:qwerty -out ca-secret.key 4096
```

* Remove passphrase
```
openssl rsa -passin pass:qwerty -in ca-secret.key -out ca.key
```

* Create CA self-signed certificate
```
openssl req -new -x509 -days 3650 -subj '/C=BY/ST=Belarus/L=Minsk/O=Example root CA/OU=Example CA unit/CN=example.com' -key ca.key -out ca.crt -config openssl.cfg
```

* Convert CA self-signed certificate to PKCS
```
openssl pkcs12 -clcerts -export -passout pass:qwerty -in ca.crt -inkey ca.key -out ca.p12
```

* Convert CA self-signed certificate to PEM
```
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in ca.p12 -out ca.pem
```

## SSL Server certificate

* Create private key for the server
```
openssl genrsa -des3 -passout pass:qwerty -out server-secret.key 4096
```

* Remove passphrase
```
openssl rsa -passin pass:qwerty -in server-secret.key -out server.key
```

* Create CSR for the server
```
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example server/OU=Example server unit/CN=server.example.com' -key server.key -out server.csr -config openssl.cfg
```

* Create certificate for the server
```
openssl x509 -req -days 3650 -in server.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out server.crt
```

* Convert the server certificate to PKCS
```
openssl pkcs12 -clcerts -export -passout pass:qwerty -in server.crt -inkey server.key -out server.p12
```

* Convert the server certificate to PEM
```
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in server.p12 -out server.pem
```

## SSL Client certificate

* Create private key for the client
```
openssl genrsa -des3 -passout pass:qwerty -out client-secret.key 4096
```

* Remove passphrase
```
openssl rsa -passin pass:qwerty -in client-secret.key -out client.key
```

* Create CSR for the client
```
openssl req -new -subj '/C=BY/ST=Belarus/L=Minsk/O=Example client/OU=Example client unit/CN=client.example.com' -key client.key -out client.csr -config openssl.cfg
```

* Create the client certificate
```
openssl x509 -req -days 3650 -in client.csr -CA ca.crt -CAkey ca.key -set_serial 01 -out client.crt
```

* Convert the client certificate to PKCS
```
openssl pkcs12 -clcerts -export -passout pass:qwerty -in client.crt -inkey client.key -out client.p12
```

* Convert the client certificate to PEM
```
openssl pkcs12 -clcerts -passin pass:qwerty -passout pass:qwerty -in client.p12 -out client.pem
```

## Diffie-Hellman key exchange

* Create DH parameters
```
openssl dhparam -out dh4096.pem 4096
```
