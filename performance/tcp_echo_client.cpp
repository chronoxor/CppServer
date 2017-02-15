#include "server/asio/service.h"
#include "server/asio/tcp_client.h"
#include "system/cpu.h"
#include "threads/thread.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "../../modules/cpp-optparse/OptionParser.h"

int message_size = 1;

std::atomic<size_t> errors(0);
std::atomic<size_t> connected(0);
std::atomic<size_t> transfered(0);

class EchoClient : public CppServer::Asio::TCPClient
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

protected:
    void onConnected() override
    {
        ++connected;

        // Send the first message
        SendMessage();
    }

    void onDisconnected() override
    {
        // Wait for a while...
        CppCommon::Thread::Sleep(1000);

        // Try to connect again
        Connect();
    }

    size_t onReceived(const void* buffer, size_t size) override
    {
        // Wait for the whole message
        if (size < message_size)
            return 0;

        // Validate the message
        const uint8_t* message = (const uint8_t*)buffer;
        for (int i = 0; i < message_size; ++i)
        {
            if (message[i] != (i % 256))
            {
                ++errors;
                break;
            }
        }

        // Update transfered statistics
        transfered += message_size;

        // Send the next message
        SendMessage();

        // Inform that we handled the whole message
        return message_size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        ++errors;
    }

private:
    std::vector<uint8_t> message;

    void SendMessage()
    {
        // Resize the message buffer
        message.resize(message_size);

        // Fill the message buffer
        for (int i = 0; i < message.size(); ++i)
            message[i] = i % 256;

        Send(message.data(), message.size());
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(1111).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CppCommon::CPU::LogicalCores()).help("Threads count. Default: %default");
    parser.add_option("-c", "--clients").action("store").type("int").set_default(100).help("Clients count. Default: %default");
    parser.add_option("-s", "--size").action("store").type("int").set_default(32).help("Message size. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // TCP server port
    std::string address = options.get("address");
    int port = options.get("port");
    message_size = std::min(message_size, (int)options.get("size"));

    std::cout << "TCP server address: " << address << std::endl;
    std::cout << "TCP server port: " << port << std::endl;
    std::cout << "Press Enter to stop all clients..." << std::endl;

    // Create Asio services
    std::vector<std::shared_ptr<CppServer::Asio::Service>> services;
    for (int i = 0; i < (int)options.get("threads"); ++i)
    {
        auto service = std::make_shared<CppServer::Asio::Service>();
        services.emplace_back(service);
    }

    // Start Asio services
    for (auto& service : services)
        service->Start();

    // Create TCP echo clients
    std::vector<std::shared_ptr<EchoClient>> clients;
    for (int i = 0; i < (int)options.get("clients"); ++i)
    {
        auto client = std::make_shared<EchoClient>(services[i % services.size()], address, port);
        clients.emplace_back(client);
    }

    // Connect clients
    for (auto& client : clients)
        client->Connect();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;
    }

    // Disconnect clients
    for (auto& client : clients)
        client->Disconnect();

    // Stop Asio services
    for (auto& service : services)
        service->Stop();

    std::cout << "Clients statistics: " << std::endl;
    std::cout << "- Caught errors: " << errors << std::endl;
    std::cout << "- Connected clients: " << connected << std::endl;
    std::cout << "- Bytes transfered: " << transfered << std::endl;

    return 0;
}
