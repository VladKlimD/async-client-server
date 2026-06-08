#include "client.h"

#include <utility>
#include <iostream>

namespace { std::mutex g_printMutex; }

Client::Client(const std::size_t id, const std::size_t commandsCount, CommandHandler commandHandler)
    : m_id { id },
    m_commandsCount { commandsCount },
    m_commandHandler { std::move(commandHandler) }
{}

void Client::run()
{
    for (std::size_t i = 0; i < m_commandsCount; ++i)
    {
        const auto sentAt { std::chrono::steady_clock::now() };

        m_commandHandler(makeCommand(i), [this, sentAt](const CommandResult& result)
        {
            handleResult(sentAt, result);
        });
    }

    std::unique_lock<std::mutex> lock { m_mutex };
    m_conditionVariable.wait(lock, [this]() { return m_completedCommands == m_commandsCount; });

    {
        const auto averageTime { m_totalTime / static_cast<int64_t>(m_commandsCount) };
        std::lock_guard<std::mutex> printMutex { g_printMutex };

        std::cout
            << "Client " << m_id << " statistics: "
            << "min=" << m_minTime.count() << " ms, "
            << "max=" << m_maxTime.count() << " ms, "
            << "avg=" << averageTime.count() << " ms\n";
    }
}

Command Client::makeCommand(const std::size_t index) const
{
    auto type { CommandType::Fast };

    switch (index % 3)
    {
        case 0:
            type = CommandType::Fast;
            break;
        case 1:
            type = CommandType::Medium;
            break;
        default:
            type = CommandType::Slow;
            break;
    }

    return Command
    {
        type,
        "client " + std::to_string(m_id) + ", command " + std::to_string(index + 1)
    };
}

void Client::handleResult(const std::chrono::steady_clock::time_point sentAt, const CommandResult&)
{
    const auto receivedAt { std::chrono::steady_clock::now() };
    const auto responseTime
    {
        std::chrono::duration_cast<std::chrono::milliseconds>(receivedAt - sentAt)
    };

    std::lock_guard<std::mutex> lock { m_mutex };

    if (m_completedCommands == 0)
    {
        m_minTime = responseTime;
        m_maxTime = responseTime;
    }
    else
    {
        if (responseTime < m_minTime)
            m_minTime = responseTime;

        if (responseTime > m_maxTime)
            m_maxTime = responseTime;
    }

    m_totalTime += responseTime;
    ++m_completedCommands;

    m_conditionVariable.notify_one();
}
