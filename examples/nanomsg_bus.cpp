/*!
    \file nanomsg_bus.cpp
    \brief Nanomsg bus example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/bus_link.h"
#include "server/nanomsg/bus_node.h"

#include <iostream>
#include <memory>

int main(int argc, char** argv)
{
    // Nanomsg bus nodes addresses
    std::string address1 = "inproc://node1";
    std::string address2 = "inproc://node2";
	std::string address3 = "inproc://node3";
	std::string address4 = "inproc://node4";

    // Create Nanomsg bus nodes
    auto node1 = std::make_shared<CppServer::Nanomsg::BusNode>(address1, false);
    auto node2 = std::make_shared<CppServer::Nanomsg::BusNode>(address2, false);
	auto node3 = std::make_shared<CppServer::Nanomsg::BusNode>(address3, false);
	auto node4 = std::make_shared<CppServer::Nanomsg::BusNode>(address4, false);

	// Start Nanomsg bus nodes
	node1->Start();
	node2->Start();
	node3->Start();
	node4->Start();

    // Link Nanomsg bus nodes
    node1->Link(address2);
	node1->Link(address3);
	node2->Link(address3);
	node2->Link(address4);
	node3->Link(address4);
	node4->Link(address1);

    // Send messages
    node1->Send("node1");
    node2->Send("node2");
	node3->Send("node3");
	node4->Send("node4");

    // Receive messages
    CppServer::Nanomsg::Message message;
	for (int i = 0; i < 3; ++i)
	{
		node1->Receive(message);
		std::cout << "node1 received: " << message << std::endl;
		node2->Receive(message);
		std::cout << "node2 received: " << message << std::endl;
		node3->Receive(message);
		std::cout << "node3 received: " << message << std::endl;
		node4->Receive(message);
		std::cout << "node4 received: " << message << std::endl;
	}

	// Stop Nanomsg bus nodes
	node1->Stop();
	node2->Stop();
	node3->Stop();
	node4->Stop();

    return 0;
}
