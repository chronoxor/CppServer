//
// Created by Ivan Shynkarenka on 16.03.2017
//

#include "benchmark/reporter_console.h"
#include "server/asio/service.h"
#include "server/asio/web_ssl_client.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "../../modules/cpp-optparse/OptionParser.h"

using namespace CppServer::Asio;

std::vector<uint8_t> message;

uint64_t timestamp_start = 0;
uint64_t timestamp_stop = 0;

std::atomic<size_t> total_errors(0);
std::atomic<size_t> total_bytes(0);
std::atomic<size_t> total_messages(0);

void SendRequest(std::shared_ptr<WebClient>& client, const std::string& uri, int messages)
{
    if (messages-- <= 0)
        return;

    // Create and fill Web request
    auto request = std::make_shared<restbed::Request>(restbed::Uri(uri));
    request->set_method("POST");
    request->set_header("Content-Length", std::to_string(message.size()));
    request->set_body(message);
    auto response = client->SendAsync(request, [&client, &uri, messages](const std::shared_ptr<restbed::Request>& request, const std::shared_ptr<restbed::Response>& response)
    {
        auto length = response->get_header("Content-Length", 0);
        WebClient::Fetch(response, length);
        timestamp_stop = CppCommon::Timestamp::nano();
        total_bytes += response->get_body().size();
        ++total_messages;

        // Dispatch a next request
        client->service()->Dispatch([&client, &uri, messages]() { SendRequest(client, uri, messages); });
    });
}

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("127.0.0.1").help("Server address. Default: %default");
    parser.add_option("-p", "--port").action("store").type("int").set_default(9000).help("Server port. Default: %default");
    parser.add_option("-t", "--threads").action("store").type("int").set_default(CppCommon::CPU::LogicalCores()).help("Count of working threads. Default: %default");
    parser.add_option("-c", "--clients").action("store").type("int").set_default(100).help("Count of working clients. Default: %default");
    parser.add_option("-m", "--messages").action("store").type("int").set_default(1000).help("Count of messages to send. Default: %default");
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

    // Web server uri
    const std::string uri = "https://" + address + ":" + std::to_string(port) + "/storage/test";

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Server port: " << port << std::endl;
    std::cout << "Server uri: " << uri << std::endl;
    std::cout << "Working threads: " << threads_count << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;
    std::cout << "Messages to send: " << messages_count << std::endl;
    std::cout << "Message size: " << message_size << std::endl;

    // Prepare a message to send
    message.resize(message_size, 0);

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
    std::vector<std::shared_ptr<WebClient>> clients;
    for (int i = 0; i < clients_count; ++i)
    {
        auto client = std::make_shared<WebClient>(services[i % services.size()]);
        clients.emplace_back(client);
    }

    timestamp_start = CppCommon::Timestamp::nano();

    // Wait for processing all messages
    std::cout << "Processing...";
    for (auto& client : clients)
        SendRequest(client, uri, messages_count / clients_count);
    while (total_messages < messages_count)
        CppCommon::Thread::Yield();
    std::cout << "Done!" << std::endl;

    // Stop Asio services
    std::cout << "Asio services stopping...";
    for (auto& service : services)
        service->Stop();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    std::cout << "Round-trip time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total bytes: " << total_bytes << std::endl;
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Bytes throughput: " << total_bytes * 1000000000 / (timestamp_stop - timestamp_start) << " bytes per second" << std::endl;
    std::cout << "Messages throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " messages per second" << std::endl;
    std::cout << "Errors: " << total_errors << std::endl;

    return 0;
}
