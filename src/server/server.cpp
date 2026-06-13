#include "server.h"

#include <cmath>
#include <utility>

Server::Server(std::size_t workersCount)
{
    if (workersCount == 0)
    {
        workersCount = 1;
    }

    m_workers.reserve(workersCount);

    for (std::size_t i = 0; i < workersCount; ++i)
    {
        // Запускаем основные worker loop для каждого потока
        m_workers.emplace_back(&Server::workerLoop, this);
    }
}

Server::~Server()
{
    {
        std::lock_guard<std::mutex> lock { m_mutex };
        m_stopped = true;
    }

    m_conditionVariable.notify_all();

    // Ожидаем завершения всех worker потоков
    for (std::thread& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void Server::calculateSinSlow(const CommandContext context, double angleRadians, SinSlowResultCallback callback)
{
    enqueue(context, [this, angleRadians, callback = std::move(callback)](const CommandContext taskContext) mutable
    {
        executeSinSlow(taskContext, angleRadians, callback);
    });
}

void Server::calculateCosMedium(const CommandContext context, double angleRadians, CosMediumResultCallback callback)
{
    enqueue(context, [this, angleRadians, callback = std::move(callback)](const CommandContext taskContext) mutable
    {
        executeCosMedium(taskContext, angleRadians, callback);
    });
}

void Server::executeFastCommand(const CommandContext context, ResultCallback callback)
{
    enqueue(context, [this, callback = std::move(callback)](CommandContext taskContext) mutable
    {
        executeFast(taskContext, callback);
    });
}

void Server::enqueue(const CommandContext context, TaskHandler handler)
{
    {
        std::lock_guard<std::mutex> lock { m_mutex };

        // Проверка что сервер не остановлен
        if (m_stopped)
        {
            return;
        }

        // Складываем задачу в очередь
        m_queue.push(Task { context, std::move(handler) });
    }

    // Будим один worker поток
    m_conditionVariable.notify_one();
}

void Server::workerLoop()
{
    while (true)
    {
        Task task;

        {
            std::unique_lock<std::mutex> lock { m_mutex };

            // Ожидаем появление задачи
            m_conditionVariable.wait(lock, [this]()
            {
                return m_stopped || !m_queue.empty();
            });

            if (m_stopped && m_queue.empty())
            {
                return;
            }

            task = std::move(m_queue.front());
            m_queue.pop();
        }

        // Выполняем задачу
        task.handler(task.context);
    }
}

void Server::executeSinSlow(const CommandContext context, const double angleRadians,
    const SinSlowResultCallback& callback)
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 1000 });
    const double result { std::sin(angleRadians) };
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

    if (callback)
    {
        callback(commandResult, result);
    }
}

void Server::executeCosMedium(const CommandContext context, const double angleRadians,
    const CosMediumResultCallback& callback)
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 500 });
    const double result { std::cos(angleRadians) };
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

    if (callback)
    {
        callback(commandResult, result);
    }
}

void Server::executeFast(const CommandContext context, const ResultCallback& callback)
{
    const auto startedAt { std::chrono::steady_clock::now() };

    std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
    const CommandResult commandResult { makeCommandResult(context, startedAt) };

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
