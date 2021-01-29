//
// Created by Ivan Shynkarenka on 02.05.2019
//

#include "server/asio/service.h"
#include "server/http/http_client.h"

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
using namespace CppServer::HTTP;

std::atomic<uint64_t> timestamp_start(Timestamp::nano());
std::atomic<uint64_t> timestamp_stop(Timestamp::nano());

std::atomic<uint64_t> total_errors(0);
std::atomic<uint64_t> total_bytes(0);
std::atomic<uint64_t> total_messages(0);

class HTTPTraceClient : public HTTPClient
{
public:
    HTTPTraceClient(const std::shared_ptr<Service>& service, const std::string& address, int port, int messages)
        : HTTPClient(service, address, port),
          _messages(messages)
    {
    }

    void SendMessage() { SendRequestAsync(request().MakeTraceRequest("/")); }

protected:
    void onConnected() override
    {
        for (size_t i = _messages; i > 0; --i)
            SendMessage();
    }

    void onSent(size_t sent, size_t pending) override
    {
        _sent += sent;
        HTTPClient::onSent(sent, pending);
    }

    void onReceived(const void* buffer, size_t size) override
    {
        _received += size;
        timestamp_stop = Timestamp::nano();
        total_bytes += size;
        HTTPClient::onReceived(buffer, size);
    }

    void onReceivedResponse(const HTTPResponse& response) override
    {
        if (response.status() == 200)
            ++total_messages;
        else
            ++total_errors;
        SendMessage();
    }

    void onReceivedResponseError(const HTTPResponse& response, const std::string& error) override
    {
        std::cout << "Response error: " << error << std::endl;
        ++total_errors;
        SendMessage();
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "HTTP Trace client caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
        ++total_errors;
    }

private:
    size_t _sent{0};
    size_t _received{0};
    size_t _messages{0};
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-a", "--address").dest("address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").dest("port").action("store").type("int").set_default(8080).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").dest("threads").action("store").type("int").set_default(CPU::PhysicalCores()).help("Count of working threads. Default: %default");
    parser.add_option("-c", "--clients").dest("clients").action("store").type("int").set_default(100).help("Count of working clients. Default: %default");
    parser.add_option("-m", "--messages").dest("messages").action("store").type("int").set_default(1).help("Count of messages to send at the same time. Default: %default");
    parser.add_option("-z", "--seconds").dest("seconds").action("store").type("int").set_default(10).help("Count of seconds to benchmarking. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        return 0;
    }

    // Client parameters
    std::string address(options.get("address"));
    int port = options.get("port");
    int threads_count = options.get("threads");
    int clients_count = options.get("clients");
    int messages_count = options.get("messages");
    int seconds_count = options.get("seconds");

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;
    std::cout << "Working messages: " << messages_count << std::endl;
    std::cout << "Seconds to benchmarking: " << seconds_count << std::endl;

    std::cout << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>(threads_count);

    // Start the Asio service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create HTTP Trace clients
    std::vector<std::shared_ptr<HTTPTraceClient>> clients;
    for (int i = 0; i < clients_count; ++i)
    {
        // Create echo client
        auto client = std::make_shared<HTTPTraceClient>(service, address, port, messages_count);
        // client->SetupNoDelay(true);
        clients.emplace_back(client);
    }

    timestamp_start = Timestamp::nano();

    // Connect clients
    std::cout << "Clients connecting...";
    for (auto& client : clients)
        client->ConnectAsync();
    std::cout << "Done!" << std::endl;
    for (const auto& client : clients)
        while (!client->IsConnected())
            Thread::Yield();
    std::cout << "All clients connected!" << std::endl;

    // Wait for benchmarking
    std::cout << "Benchmarking...";
    Thread::Sleep(seconds_count * 1000);
    std::cout << "Done!" << std::endl;

    // Disconnect clients
    std::cout << "Clients disconnecting...";
    for (auto& client : clients)
        client->DisconnectAsync();
    std::cout << "Done!" << std::endl;
    for (const auto& client : clients)
        while (client->IsConnected())
            Thread::Yield();
    std::cout << "All clients disconnected!" << std::endl;

    // Stop the Asio service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    std::cout << "Errors: " << total_errors << std::endl;

    std::cout << std::endl;

    std::cout << "Total time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total data: " << CppBenchmark::ReporterConsole::GenerateDataSize(total_bytes) << std::endl;
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Data throughput: " << CppBenchmark::ReporterConsole::GenerateDataSize(total_bytes * 1000000000 / (timestamp_stop - timestamp_start)) << "/s" << std::endl;
    if (total_messages > 0)
    {
        std::cout << "Message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
        std::cout << "Message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " msg/s" << std::endl;
    }

    return 0;
}
