#pragma once

#include "server.h"

#include <functional>
#include <string>

class ServerNetwork
{
public:
    using ResponseHandler = std::function<void(std::string)>;

    ServerNetwork(Server& server, ResponseHandler responseHandler);

    // Принимает пакет в виде строки через FakeTransport
    void receive(const std::string& data) const;

private:
    struct Request
    {
        Server::CommandContext context;
        std::string commandName;
        double argument {};
    };

    // Парсинг запроса принятого от клиента
    static bool parse(const std::string& data, Request& request, std::string& error);
    void handleRequest(const Request& request) const;
    // Формирование и отправка ошибки если парсинг запроса завершился неудачно
    void sendError(Server::CommandContext context, const std::string& error) const;
    // Отправка ответа о завершении команды без аргументов
    void sendResponse(const Server::CommandResult& result) const;
    // Отправка ответа о завершении команды с аргументом (в нашем случае для вычисления cos и sin)
    void sendResponse(const Server::CommandResult& result, double value) const;

    Server& m_server;
    ResponseHandler m_responseHandler;
};
