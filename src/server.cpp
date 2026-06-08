#include "server.h"

Server::Server(std::size_t workersCount)
{
    if (workersCount == 0)
    {
        workersCount = 1;
    }

    m_workers.reserve(workersCount);

    for (std::size_t i = 0; i < workersCount; ++i)
    {
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

    for (std::thread& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void Server::executeCommand(Command command, ResultCallback callback)
{
    {
        std::lock_guard<std::mutex> lock { m_mutex };

        if (m_stopped)
        {
            return;
        }

        m_queue.push(Task { std::move(command), std::move(callback) });
    }

    m_conditionVariable.notify_one();
}

void Server::workerLoop()
{
    while (true)
    {
        Task task;

        {
            std::unique_lock<std::mutex> lock { m_mutex };

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

        const CommandResult result { processCommand(task.command) };
        task.callback(result);
    }
}

CommandResult Server::processCommand(const Command& command)
{
    const auto startedAt { std::chrono::steady_clock::now() };

    switch (command.type)
    {
        case CommandType::Fast:
            std::this_thread::sleep_for(std::chrono::milliseconds { 100 });
            break;

        case CommandType::Medium:
            std::this_thread::sleep_for(std::chrono::milliseconds { 500 });
            break;

        case CommandType::Slow:
            std::this_thread::sleep_for(std::chrono::milliseconds { 1000 });
            break;
    }

    const auto finishedAt { std::chrono::steady_clock::now() };

    return CommandResult
    {
        command.type,
        command.data,
        std::chrono::duration_cast<std::chrono::milliseconds>(finishedAt - startedAt)
    };
}
