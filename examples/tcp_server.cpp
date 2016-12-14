/*!
    \file tcp_server.cpp
    \brief TCP server example
    \author Ivan Shynkarenka
    \date 14.12.2016
    \copyright MIT License
*/

#include "server/tcp/server.h"
#include "server/tcp/session.h"

#include <iostream>

class MyServer;
class MySession;

class MyServer : public CppServer::TCPServer<MyServer, MySession>
{
public:
    using CppServer::TCPServer<MyServer, MySession>::TCPServer;

protected:
    void onStarting() override { std::cout << "MyServer starting..." << std::endl; }
    void onStarted() override { std::cout << "MyServer started!" << std::endl; }
    void onStopping() override { std::cout << "MyServer stopping..." << std::endl; }
    void onStopped() override { std::cout << "MyServer stopped!" << std::endl; }
    void onAccepted(const MySession& session) override { std::cout << "MyServer new session is accepted!" << std::endl; }
    void onError(int error, const std::string& category, const std::string& message) override { std::cout << "MyServer caught an error with code " << error << " and category '" << category << "': " << message << std::endl; }
};

class MySession : public CppServer::TCPSession<MyServer, MySession>
{
public:
    using CppServer::TCPSession<MyServer, MySession>::TCPSession;
};

int main(int argc, char** argv)
{
    std::cout << "Press Enter to stop..." << std::endl;

    // Create a new server
    MyServer server(CppServer::TCPProtocol::IPv4, 1234);

    // Start the server
    server.Start();

    // Wait for input
    std::cin.get();

    // Stop the server
    server.Start();

    return 0;
}
