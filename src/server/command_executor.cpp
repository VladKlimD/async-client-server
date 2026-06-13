#include "command_executor.h"

#include <utility>

CommandExecutor::CommandExecutor(std::size_t workersCount)
{
    if (workersCount == 0)
    {
        workersCount = 1;
    }

    m_workers.reserve(workersCount);

    for (std::size_t i = 0; i < workersCount; ++i)
    {
        m_workers.emplace_back(&CommandExecutor::workerLoop, this);
    }
}

CommandExecutor::~CommandExecutor()
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

void CommandExecutor::enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock { m_mutex };

        if (m_stopped)
        {
            return;
        }

        m_queue.push(std::move(task));
    }

    m_conditionVariable.notify_one();
}

void CommandExecutor::workerLoop()
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

        task();
    }
}
