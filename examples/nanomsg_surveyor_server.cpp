/*!
    \file nanomsg_surveyor_server.cpp
    \brief Nanomsg surveyor server example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/surveyor_server.h"

#include <iostream>
#include <memory>

class ExampleSurveyorServer : public CppServer::Nanomsg::SurveyorServer
{
public:
    using CppServer::Nanomsg::SurveyorServer::SurveyorServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg surveyor server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg surveyor server stopped!" << std::endl;
    }

    void onSurveyStarted(const void* buffer, size_t size)
    {
        std::string question((const char*)buffer, size);
        std::cout << "Survey started! Question: " << question << std::endl;
    }

    void onSurveyStopped()
    {
        std::cout << "Survey finished!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg surveyor server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg surveyor server address
    std::string address = "tcp://*:6670";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg surveyor server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg surveyor server
    auto server = std::make_shared<ExampleSurveyorServer>(address);

    // Start the server
    server->Start();

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }

        // Survey the entered text to all respondent clients
        server->Survey(line);
    }

    // Stop the server
    server->Stop();

    return 0;
}
