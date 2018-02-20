//
// Created by Ivan Shynkarenka on 15.03.2017
//

#include "server/asio/service.h"
#include "server/asio/udp_client.h"

#include "benchmark/reporter_console.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include <OptionParser.h>

using namespace CppBenchmark;
using namespace CppCommon;
using namespace CppServer::Asio;

std::vector<uint8_t> message_to_send;

std::atomic<uint64_t> timestamp_start(0);
std::atomic<uint64_t> timestamp_stop(0);

std::atomic<uint64_t> total_errors(0);
std::atomic<uint64_t> total_bytes(0);
std::atomic<uint64_t> total_messages(0);

class EchoClient : public UDPClient
{
public:
    explicit EchoClient(std::shared_ptr<Service> service, const std::string& address, int port, int messages)
        : UDPClient(service, address, port),
          _connected(false)
    {
        _messages = messages;
    }

    bool connected() const noexcept { return _connected; }

protected:
    void onConnected() override
    {
        _connected = true;

        SendMessage();
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override
    {
        timestamp_stop = Timestamp::nano();
        total_bytes += size;
        ++total_messages;

        SendMessage();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        ++total_errors;
    }

private:
    std::atomic<bool> _connected;
    int _messages;

    void SendMessage()
    {
        if (_messages-- > 0)
            Send(message_to_send.data(), message_to_send.size());
        else
            Disconnect();
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(3333).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CPU::PhysicalCores()).help("Count of working threads. Default: %default");
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

    // Client parameters
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
    message_to_send.resize(message_size, 0);

    // Create Asio services
    std::vector<std::shared_ptr<Service>> services;
    for (int i = 0; i < threads_count; ++i)
    {
        auto service = std::make_shared<Service>();
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
        auto client = std::make_shared<EchoClient>(services[i % services.size()], address, port, messages_count / clients_count);
        client->SetupReuseAddress(true);
        client->SetupReusePort(true);
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

    // Wait for processing all messages
    std::cout << "Processing...";
    for (auto& client : clients)
    {
        while (client->IsConnected())
            Thread::Sleep(100);
    }
    std::cout << "Done!" << std::endl;

    // Stop Asio services
    std::cout << "Asio services stopping...";
    for (auto& service : services)
        service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    std::cout << "Errors: " << total_errors << std::endl;

    std::cout << std::endl;

    std::cout << "Round-trip time: " << ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total bytes: " << total_bytes << std::endl;
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Bytes throughput: " << total_bytes * 1000000000 / (timestamp_stop - timestamp_start) << " bytes per second" << std::endl;
    std::cout << "Message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "Message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " messages per second" << std::endl;

    return 0;
}
