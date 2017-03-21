//
// Created by Ivan Shynkarenka on 06.02.2017
//

#include "catch.hpp"

#include "server/nanomsg/bus.h"
#include "threads/thread.h"

#include <atomic>
#include <memory>

using namespace CppCommon;
using namespace CppServer::Nanomsg;

TEST_CASE("Nanomsg bus", "[CppServer][Nanomsg]")
{
    // Nanomsg bus nodes addresses
    std::string address1 = "inproc://node1";
    std::string address2 = "inproc://node2";
    std::string address3 = "inproc://node3";
    std::string address4 = "inproc://node4";

    // Create Nanomsg bus nodes
    auto node1 = std::make_shared<Bus>(address1, false);
    auto node2 = std::make_shared<Bus>(address2, false);
    auto node3 = std::make_shared<Bus>(address3, false);
    auto node4 = std::make_shared<Bus>(address4, false);

    // Start Nanomsg bus nodes
    REQUIRE(node1->Start());
    REQUIRE(node2->Start());
    REQUIRE(node3->Start());
    REQUIRE(node4->Start());

    // Link Nanomsg bus nodes
    REQUIRE(node1->Link(address2));
    REQUIRE(node1->Link(address3));
    REQUIRE(node2->Link(address3));
    REQUIRE(node2->Link(address4));
    REQUIRE(node3->Link(address4));
    REQUIRE(node4->Link(address1));

    // Send messages
    REQUIRE(node1->Send("node1") == 5);
    REQUIRE(node2->Send("node2") == 5);
    REQUIRE(node3->Send("node3") == 5);
    REQUIRE(node4->Send("node4") == 5);

    // Receive messages
    Message message;
    for (int i = 0; i < 3; ++i)
    {
        REQUIRE(node1->TryReceive(message) == 5);
        REQUIRE(node2->TryReceive(message) == 5);
        REQUIRE(node3->TryReceive(message) == 5);
        REQUIRE(node4->TryReceive(message) == 5);
    }
    REQUIRE(node1->TryReceive(message) == 0);
    REQUIRE(node2->TryReceive(message) == 0);
    REQUIRE(node3->TryReceive(message) == 0);
    REQUIRE(node4->TryReceive(message) == 0);

    // Stop Nanomsg bus nodes
    REQUIRE(node1->Stop());
    REQUIRE(node2->Stop());
    REQUIRE(node3->Stop());
    REQUIRE(node4->Stop());
}
