#include "client/client.h"
#include "fake_transport.h"
#include "options_parser.hpp"
#include "server/server.h"
#include "server/server_network.h"

#include <iostream>
#include <vector>
#include <thread>
#include <memory>

int main(int argc, char* argv[])
{
    ProgramOptions programOptions;

    // Парсинг аргументов
    if (!parseProgramOptions(argc, argv, programOptions))
    {
        return 1;
    }

    std::cout
        << "Clients count: " << programOptions.clientsCount << '\n'
        << "Commands per client: " << programOptions.commandsPerClient << '\n';

    // Создаем сервер, передаем максимальное количество потоков
    Server server { std::thread::hardware_concurrency() };

    // Имитация сетевого транспорта
    FakeTransport transport;

    // Коллбэк отправки ответа клиенту
    ServerNetwork::ResponseHandler responseHandler { [&transport](const std::string& response)
        { transport.sendToClient(response); }};
    // Серверная сетевая прослойка
    ServerNetwork serverNetwork { server, responseHandler };

    // Коллбэк отправки запроса серверу
    FakeTransport::ServerHandler serverHandler { [&serverNetwork](const std::string& request)
        { serverNetwork.receive(request); }};
    transport.setServerHandler(serverHandler);

    std::vector<std::unique_ptr<Client>> clients;
    std::vector<std::thread> clientsThreads;

    clients.reserve(programOptions.clientsCount);
    clientsThreads.reserve(programOptions.clientsCount);

    // Создаем клиенты динамически, чтобы избежать перемещения (mutex и cv внутри)
    for (std::size_t i = 0; i < programOptions.clientsCount; ++i)
    {
        clients.emplace_back(new Client
        {
            i + 1,
            programOptions.commandsPerClient,
            transport
        });
    }

    // Создаем потоки для каждого клиента и запускаем их в работу
    for (std::unique_ptr<Client>& client : clients)
    {
        clientsThreads.emplace_back(&Client::run, client.get());
    }

    // Дожидаемся окончания работы клиентов
    for (std::thread& thread : clientsThreads)
    {
        thread.join();
    }

    return 0;
}
