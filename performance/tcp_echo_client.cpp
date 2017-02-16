#include "benchmark/reporter_console.h"
#include "server/asio/service.h"
#include "server/asio/tcp_client.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "../../modules/cpp-optparse/OptionParser.h"

std::atomic<size_t> total_errors(0);
std::atomic<size_t> total_received(0);

class EchoClient : public CppServer::Asio::TCPClient
{
public:
    using CppServer::Asio::TCPClient::TCPClient;

protected:
    size_t onReceived(const void* buffer, size_t size) override
    {
        total_received += size;
        return size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        ++total_errors;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(1111).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CppCommon::CPU::LogicalCores()).help("Count of working threads. Default: %default");
    parser.add_option("-c", "--clients").action("store").type("int").set_default(100).help("Count of working clients. Default: %default");
    parser.add_option("-m", "--messages").action("store").type("int").set_default(1000000).help("Count of messages to send. Default: %default");
    parser.add_option("-s", "--size").action("store").type("int").set_default(32).help("Single message size. Default: %default");

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
    int messages_count = options.get("messages");
    int message_size = options.get("size");

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;
    std::cout << "Messages to send: " << messages_count << std::endl;
    std::cout << "Message size: " << message_size << std::endl;

    // Prepare a message to send
    std::vector<uint8_t> message(message_size, 0);

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

    uint64_t timestamp_start = CppCommon::Timestamp::nano();

    // Send messages to the server
    for (int i = 0; i < messages_count; ++i)
        clients[i % clients.size()]->Send(message.data(), message.size());

    uint64_t timestamp_sent = CppCommon::Timestamp::nano();

    // Wait for received data
    size_t total_bytes_sent = messages_count * message_size;
    while (total_received < total_bytes_sent)
        CppCommon::Thread::Yield();

    uint64_t timestamp_received = CppCommon::Timestamp::nano();

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

    std::cout << "Processing time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_received - timestamp_start) << std::endl;
    std::cout << "Processing bytes: " << total_received << std::endl;
    std::cout << "Processing messages: " << total_received / message_size << std::endl;
    std::cout << "Bytes throughput: " << total_received * 1000000000 / (timestamp_received - timestamp_start) << " bytes per second" << std::endl;
    std::cout << "Messages throughput: " << (total_received / message_size) * 1000000000 / (timestamp_received - timestamp_start) << " messages per second" << std::endl;
    std::cout << "Errors: " << total_errors << std::endl;

    return 0;
}
