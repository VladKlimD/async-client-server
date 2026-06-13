#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <thread>

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

    using ResultCallback = std::function<void(CommandResult)>;
    using SinSlowResultCallback = std::function<void(CommandResult, double)>;
    using CosMediumResultCallback = std::function<void(CommandResult, double)>;

    explicit Server(std::size_t workersCount);
    ~Server();

    void calculateSinSlow(CommandContext context, double angleRadians, SinSlowResultCallback callback);
    void calculateCosMedium(CommandContext context, double angleRadians, CosMediumResultCallback callback);
    void executeFastCommand(CommandContext context, ResultCallback callback);

private:
    using TaskHandler = std::function<void(CommandContext)>;

    void enqueue(CommandContext context, TaskHandler handler);
    void workerLoop();
    void executeSinSlow(CommandContext context, double angleRadians, const SinSlowResultCallback& callback);
    void executeCosMedium(CommandContext context, double angleRadians, const CosMediumResultCallback& callback);
    void executeFast(CommandContext context, const ResultCallback& callback);
    static CommandResult makeCommandResult(CommandContext context, std::chrono::steady_clock::time_point startedAt);

    struct Task
    {
        CommandContext context;
        TaskHandler handler;
    };

    // Очередь задач
    std::queue<Task> m_queue;
    // Защита очереди и флага остановки
    std::mutex m_mutex;
    // Ожидание задач worker потоками
    std::condition_variable m_conditionVariable;
    // Пул рабочих потоков
    std::vector<std::thread> m_workers;
    // Признак остановки сервера
    bool m_stopped { false };
};
