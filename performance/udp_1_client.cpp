#include "benchmark/reporter_console.h"
#include "server/asio/service.h"
#include "server/asio/udp_client.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "../../modules/cpp-optparse/OptionParser.h"

std::atomic<size_t> total_errors(0);
std::atomic<size_t> total_received(0);

class EchoClient : public CppServer::Asio::UDPClient
{
public:
    using CppServer::Asio::UDPClient::UDPClient;

protected:
    void onConnected() override
    {
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

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        // Update transfered statistics
        total_received += size;

        // Send the next message
        SendMessage();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        ++total_errors;
    }

private:
    std::vector<uint8_t> message;

    void SendMessage()
    {
        // Resize the message buffer
        message.resize(32);
        Send(message.data(), message.size());
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(2222).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CppCommon::CPU::LogicalCores()).help("Count of working threads. Default: %default");
    parser.add_option("-c", "--clients").action("store").type("int").set_default(100).help("Count of working clients. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Server address and port
    std::string address(options.get("address"));
    int port = options.get("port");
    int threads_count = options.get("threads");
    int clients_count = options.get("clients");

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;

    // Create Asio services
    std::vector<std::shared_ptr<CppServer::Asio::Service>> services;
    for (int i = 0; i < threads_count; ++i)
    {
        auto service = std::make_shared<CppServer::Asio::Service>();
        services.emplace_back(service);
    }

    // Start Asio services
    std::cout << "Asio services starting...";
    for (auto& service : services)
        service->Start();
    std::cout << "Done!" << std::endl;

    // Create echo clients
    std::vector<std::shared_ptr<EchoClient>> clients;
    for (int i = 0; i < clients_count; ++i)
    {
        auto client = std::make_shared<EchoClient>(services[i % services.size()], address, port);
        clients.emplace_back(client);
    }

    // Connect clients
    std::cout << "Clients connecting...";
    for (auto& client : clients)
    {
        client->Connect();
        while (!client->IsConnected())
            CppCommon::Thread::Yield();
    }
    std::cout << "Done!" << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        std::cout << "Processing bytes: " << total_received << std::endl;
        std::cout << "Errors: " << total_errors << std::endl;
    }

    // Disconnect clients
    std::cout << "Clients disconnecting...";
    for (auto& client : clients)
        client->Disconnect();
    std::cout << "Done!" << std::endl;

    // Stop Asio services
    std::cout << "Asio services stopping...";
    for (auto& service : services)
        service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    return 0;
}
