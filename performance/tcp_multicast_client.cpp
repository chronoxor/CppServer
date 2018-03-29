//
// Created by Ivan Shynkarenka on 29.03.2018
//

#include "server/asio/service.h"
#include "server/asio/tcp_client.h"

#include "benchmark/reporter_console.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include <OptionParser.h>

using namespace CppCommon;
using namespace CppServer::Asio;

std::atomic<uint64_t> timestamp_start(0);
std::atomic<uint64_t> timestamp_stop(0);

std::atomic<uint64_t> total_errors(0);
std::atomic<uint64_t> total_bytes(0);
std::atomic<uint64_t> total_messages(0);

class MulticastClient : public TCPClient
{
public:
    MulticastClient(std::shared_ptr<Service> service, const std::string& address, int port)
        : TCPClient(service, address, port),
          _connected(false)
    {
    }

    bool connected() const noexcept { return _connected; }

protected:
    void onConnected() override
    {
        _connected = true;
    }

    void onReceived(const void* buffer, size_t size) override
    {
        total_bytes += size;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        ++total_errors;
    }

private:
    std::atomic<bool> _connected;
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(1111).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CPU::PhysicalCores()).help("Count of working threads. Default: %default");
    parser.add_option("-c", "--clients").action("store").type("int").set_default(100).help("Count of working clients. Default: %default");
    parser.add_option("-s", "--size").action("store").type("int").set_default(32).help("Single message size. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Client parameters
    std::string address(options.get("address"));
    int port = options.get("port");
    int threads_count = options.get("threads");
    int clients_count = options.get("clients");
    int message_size = options.get("size");

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;
    std::cout << "Message size: " << message_size << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>(threads_count);

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create multicast clients
    std::vector<std::shared_ptr<MulticastClient>> clients;
    for (int i = 0; i < clients_count; ++i)
    {
        auto client = std::make_shared<MulticastClient>(service, address, port);
        // client->SetupNoDelay(true);
        clients.emplace_back(client);
    }

    timestamp_start = Timestamp::nano();

    // Connect clients
    std::cout << "Clients connecting...";
    for (auto& client : clients)
        client->Connect();
    std::cout << "Done!" << std::endl;
    for (auto& client : clients)
        while (!client->connected())
            Thread::Yield();
    std::cout << "All clients connected!" << std::endl;

    // Sleep for 10 seconds...
    std::cout << "Processing...";
    Thread::Sleep(10000);
    std::cout << "Done!" << std::endl;

    // Disconnect clients
    std::cout << "Clients disconnecting...";
    for (auto& client : clients)
        client->Disconnect();
    std::cout << "Done!" << std::endl;
    for (auto& client : clients)
        while (client->IsConnected())
            Thread::Yield();
    std::cout << "All clients disconnected!" << std::endl;

    timestamp_stop = Timestamp::nano();

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    std::cout << "Errors: " << total_errors << std::endl;

    std::cout << std::endl;

    total_messages = total_bytes / message_size;

    std::cout << "Multicast time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total data: " << CppBenchmark::ReporterConsole::GenerateDataSize(total_bytes) << std::endl;
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Data throughput: " << CppBenchmark::ReporterConsole::GenerateDataSize(total_bytes * 1000000000 / (timestamp_stop - timestamp_start)) << " per second" << std::endl;
    std::cout << "Message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "Message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " messages per second" << std::endl;

    return 0;
}
