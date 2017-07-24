//
// Created by Ivan Shynkarenka on 15.03.2017
//

#include "server/nanomsg/request_client.h"

#include "benchmark/reporter_console.h"
#include "system/cpu.h"
#include "threads/thread.h"
#include "time/timestamp.h"

#include <atomic>
#include <iostream>
#include <vector>

#include "../../modules/cpp-optparse/OptionParser.h"

using namespace CppBenchmark;
using namespace CppCommon;
using namespace CppServer::Nanomsg;

std::vector<uint8_t> message;

uint64_t timestamp_start = 0;
uint64_t timestamp_stop = 0;

std::atomic<uint64_t> total_errors(0);
std::atomic<uint64_t> total_bytes(0);
std::atomic<uint64_t> total_messages(0);

class EchoClient : public RequestClient
{
public:
    using RequestClient::RequestClient;

protected:
    void onReceived(Message& message) override
    {
        timestamp_stop = Timestamp::nano();
        total_bytes += message.size();
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Client caught an error with code " << error << "': " << message << std::endl;
        ++total_errors;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-a", "--address").set_default("tcp://127.0.0.1:6666").help("Server address. Default: %default");
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
    int clients_count = options.get("clients");
    int messages_count = options.get("messages");
    int message_size = options.get("size");

    std::cout << "Server address: " << address << std::endl;
    std::cout << "Working clients: " << clients_count << std::endl;
    std::cout << "Messages to send: " << messages_count << std::endl;
    std::cout << "Message size: " << message_size << std::endl;

    // Prepare a message to send
    message.resize(message_size, 0);

    // Create echo clients
    std::vector<std::shared_ptr<EchoClient>> clients;
    for (int i = 0; i < clients_count; ++i)
    {
        auto client = std::make_shared<EchoClient>(address);
        clients.emplace_back(client);
    }

    timestamp_start = Timestamp::nano();

    // Connect clients
    std::cout << "Clients connecting...";
    for (auto& client : clients)
    {
        client->Connect();
        while (!client->IsConnected())
            Thread::Yield();
    }
    std::cout << "Done!" << std::endl;

    // Wait for processing all messages
    std::cout << "Processing...";
    for (int i = 0; i < messages_count; ++i)
        clients[i % clients_count]->Request(message.data(), message.size());
    std::cout << "Done!" << std::endl;

    // Disconnect clients
    std::cout << "Clients disconnecting...";
    for (auto& client : clients)
    {
        client->Disconnect();
        while (client->IsConnected())
            Thread::Yield();
    }
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    total_messages = total_bytes / message_size;

    std::cout << "Round-trip time: " << ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total bytes: " << total_bytes << std::endl;
    std::cout << "Total messages: " << total_messages << std::endl;
    std::cout << "Bytes throughput: " << total_bytes * 1000000000 / (timestamp_stop - timestamp_start) << " bytes per second" << std::endl;
    std::cout << "Messages throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " messages per second" << std::endl;
    std::cout << "Errors: " << total_errors << std::endl;

    return 0;
}
