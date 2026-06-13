#include "server.h"
#include "request_tracker.h"

#include <cmath>
#include <memory>
#include <utility>

Server::Server(const std::size_t workersCount)
    : m_requestTracker { std::make_unique<RequestTracker>() }, m_executor { workersCount }
{}

void Server::calculateSinSlow(const CommandContext context, double angleRadians, SinSlowResultCallback callback)
{
    m_requestTracker->markPending(context, "sin_slow");

    const CommandExecutor::Task task = [this, context, angleRadians, callback = std::move(callback)]() mutable
        { executeSinSlow(context, angleRadians, callback); };

    m_executor.enqueue(task);
}

void Server::calculateCosMedium(const CommandContext context, double angleRadians, CosMediumResultCallback callback)
{
    m_requestTracker->markPending(context, "cos_medium");

    const CommandExecutor::Task task = [this, context, angleRadians, callback = std::move(callback)]() mutable
        { executeCosMedium(context, angleRadians, callback); };

    m_executor.enqueue(task);
}

void Server::executeFastCommand(const CommandContext context, ResultCallback callback)
{
    m_requestTracker->markPending(context, "fast");

    const CommandExecutor::Task task = [this, context, callback = std::move(callback)]() mutable
        { executeFast(context, callback); };

    m_executor.enqueue(task);
}

std::vector<std::uint64_t> Server::waitingClientIds() const
{
    return m_requestTracker->waitingClientIds();
}

std::vector<Server::CommandReport> Server::clientReports(const std::uint64_t clientId) const
{
    return m_requestTracker->clientReports(clientId);
}

void Server::executeSinSlow(const CommandContext context, const double angleRadians,
    const SinSlowResultCallback& callback) const
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 1000 });
    const double result { std::sin(angleRadians) };
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

    m_requestTracker->markCompleted(commandResult);

    if (callback)
    {
        callback(commandResult, result);
    }
}

void Server::executeCosMedium(const CommandContext context, const double angleRadians,
    const CosMediumResultCallback& callback) const
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 500 });
    const double result { std::cos(angleRadians) };
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

    m_requestTracker->markCompleted(commandResult);

    if (callback)
    {
        callback(commandResult, result);
    }
}

void Server::executeFast(const CommandContext context, const ResultCallback& callback) const
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

    m_requestTracker->markCompleted(commandResult);

    if (callback)
    {
        callback(commandResult);
    }
}

Server::CommandResult Server::makeCommandResult(const CommandContext context,
    const std::chrono::steady_clock::time_point startedAt)
{
    const auto finishedAt { std::chrono::steady_clock::now() };

    return CommandResult
    {
        context.clientId,
        context.requestId,
        std::chrono::duration_cast<std::chrono::milliseconds>(finishedAt - startedAt)
    };
}
