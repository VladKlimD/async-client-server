#pragma once

#include "command_executor.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class RequestTracker;

class Server
{
public:
    struct CommandContext
    {
        std::uint64_t clientId {};
        std::uint64_t requestId {};
    };

    struct CommandResult
    {
        std::uint64_t clientId {};
        std::uint64_t requestId {};
        std::chrono::milliseconds executionTime {};
    };

    enum class CommandStatus
    {
        Pending,
        Completed
    };

    struct CommandReport
    {
        CommandContext context;
        std::string commandName;
        CommandStatus status { CommandStatus::Pending };
        std::chrono::milliseconds executionTime {};
    };

    using ResultCallback = std::function<void(CommandResult)>;
    using SinSlowResultCallback = std::function<void(CommandResult, double)>;
    using CosMediumResultCallback = std::function<void(CommandResult, double)>;

    explicit Server(std::size_t workersCount);
    ~Server() = default;

    // Асинхронный расчет sin с долгим выполнением
    void calculateSinSlow(CommandContext context, double angleRadians, SinSlowResultCallback callback);
    // Асинхронный расчет cos со средним временем выполнения
    void calculateCosMedium(CommandContext context, double angleRadians, CosMediumResultCallback callback);
    // Асинхронное выполнение быстрой команды
    void executeFastCommand(CommandContext context, ResultCallback callback);

    // Возвращает клиентов с незавершенными командами
    std::vector<std::uint64_t> waitingClientIds() const;
    // Возвращает отчеты по командам клиента
    std::vector<CommandReport> clientReports(std::uint64_t clientId) const;

private:
    // Выполнение команд в отдельном worker потоке через executor
    void executeSinSlow(CommandContext context, double angleRadians, const SinSlowResultCallback& callback) const;
    void executeCosMedium(CommandContext context, double angleRadians, const CosMediumResultCallback& callback) const;
    void executeFast(CommandContext context, const ResultCallback& callback) const;
    // Формирует результат выполненной команды
    static CommandResult makeCommandResult(CommandContext context, std::chrono::steady_clock::time_point startedAt);

    std::unique_ptr<RequestTracker> m_requestTracker;
    CommandExecutor m_executor;
};
