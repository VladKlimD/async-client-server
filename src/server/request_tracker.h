#pragma once

#include "server.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class RequestTracker
{
public:
    // Сохраняет информацию о поставленной команде
    void markPending(Server::CommandContext context, const std::string& commandName);
    // Помечает команду как завершенную
    void markCompleted(const Server::CommandResult& result);

    // Возвращает клиентов с незавершенными командами
    std::vector<std::uint64_t> waitingClientIds() const;
    // Возвращает отчеты по командам клиента
    std::vector<Server::CommandReport> clientReports(std::uint64_t clientId) const;

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::uint64_t, std::map<std::uint64_t, Server::CommandReport>> m_reportsByClient;
};
