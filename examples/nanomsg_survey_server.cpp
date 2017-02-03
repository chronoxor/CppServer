/*!
    \file nanomsg_survey_server.cpp
    \brief Nanomsg survey server example
    \author Ivan Shynkarenka
    \date 03.02.2017
    \copyright MIT License
*/

#include "server/nanomsg/survey_server.h"

#include <iostream>
#include <memory>

class ExampleSurveyServer : public CppServer::Nanomsg::SurveyServer
{
public:
    using CppServer::Nanomsg::SurveyServer::SurveyServer;

protected:
    void onStarted() override
    {
        std::cout << "Nanomsg survey server started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Nanomsg survey server stopped!" << std::endl;
    }

    void onError(int error, const std::string& message) override
    {
        std::cout << "Nanomsg survey server caught an error with code " << error << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    // Nanomsg survey server address
    std::string address = "tcp://*:6670";
    if (argc > 1)
        address = std::atoi(argv[1]);

    std::cout << "Nanomsg survey server address: " << address << std::endl;
    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Create a new Nanomsg survey server
    auto server = std::make_shared<ExampleSurveyServer>(address);

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
        if (server->Send(line) == line.size())
        {
            // Start the survey
            std::cout << "Survey started! Question: " << line << std::endl;
            while (true)
            {
                CppServer::Nanomsg::Message msg;

                // Receive survey responses from clients
                std::tuple<size_t, bool> result = server->ReceiveSurvey(msg);

                // Show answers from respondents
                if (std::get<0>(result) > 0)
                {
                    std::string message((const char*)msg.buffer(), msg.size());
                    std::cout << "Answer: " << message << std::endl;
                }

                // Finish the survey
                if (std::get<1>(result))
                {
                    std::cout << "Survey finished!" << std::endl;
                    break;
                }
            }
        }
    }

    // Stop the server
    server->Stop();

    return 0;
}
