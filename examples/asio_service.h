/*!
    \file asio_service.h
    \brief Asio service example
    \author Ivan Shynkarenka
    \date 15.01.2017
    \copyright MIT License
*/

#include "server/asio/service.h"

#include <iostream>

class AsioService : public CppServer::Asio::Service
{
public:
    using CppServer::Asio::Service::Service;

protected:
    void onStarted() override
    {
        std::cout << "Asio service started!" << std::endl;
    }

    void onStopped() override
    {
        std::cout << "Asio service stopped!" << std::endl;
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Asio service caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};
