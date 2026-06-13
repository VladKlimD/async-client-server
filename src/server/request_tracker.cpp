#include "request_tracker.h"

#include <algorithm>
#include <utility>

void RequestTracker::markPending(const Server::CommandContext context, const std::string& commandName)
{
    std::lock_guard<std::mutex> lock { m_mutex };

    Server::CommandReport report;
    report.context = context;
    report.commandName = commandName;
    report.status = Server::CommandStatus::Pending;

    m_reportsByClient[context.clientId][context.requestId] = report;
}

void RequestTracker::markCompleted(const Server::CommandResult& result)
{
    std::lock_guard<std::mutex> lock { m_mutex };

    Server::CommandReport& report { m_reportsByClient[result.clientId][result.requestId] };
    report.context.clientId = result.clientId;
    report.context.requestId = result.requestId;
    report.status = Server::CommandStatus::Completed;
    report.executionTime = result.executionTime;
}

std::vector<std::uint64_t> RequestTracker::waitingClientIds() const
{
    std::vector<std::uint64_t> clientIds;
    std::lock_guard<std::mutex> lock { m_mutex };

    for (const auto& clientReports : m_reportsByClient)
    {
        const auto pendingIt
        {
            std::find_if(clientReports.second.begin(), clientReports.second.end(),
                [](const std::pair<const std::uint64_t, Server::CommandReport>& report)
                {
                    return report.second.status == Server::CommandStatus::Pending;
                })
        };

        if (pendingIt != clientReports.second.end())
        {
            clientIds.push_back(clientReports.first);
        }
    }

    std::sort(clientIds.begin(), clientIds.end());

    return clientIds;
}

std::vector<Server::CommandReport> RequestTracker::clientReports(const std::uint64_t clientId) const
{
    std::vector<Server::CommandReport> reports;
    std::lock_guard<std::mutex> lock { m_mutex };

    const auto clientIt { m_reportsByClient.find(clientId) };
    if (clientIt == m_reportsByClient.end())
    {
        return reports;
    }

    reports.reserve(clientIt->second.size());

    for (const auto& report : clientIt->second)
    {
        reports.push_back(report.second);
    }

    return reports;
}
