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
        Command command { makeCommand(i) };

        {
            std::lock_guard<std::mutex> printMutex { g_printMutex };
            std::cout
                << "Client " << m_id
                << " sent " << commandTypeToString(command.type)
                << " command: " << command.data << '\n';
        }

        const auto sentAt { std::chrono::steady_clock::now() };
        m_commandHandler(std::move(command), [this, sentAt](const CommandResult& result)
        {
            handleResult(sentAt, result);
        });
    }

    std::unique_lock<std::mutex> lock { m_mutex };
    m_conditionVariable.wait(lock, [this]() { return m_completedCommands == m_commandsCount; });

    const std::size_t completedCommands { m_completedCommands };
    const auto totalTime { m_totalTime };
    const auto minTime { m_minTime };
    const auto maxTime { m_maxTime };
    lock.unlock();

    {
        const auto averageTime
        {
            std::chrono::milliseconds
            {
                totalTime.count() / static_cast<std::chrono::milliseconds::rep>(completedCommands)
            }
        };
        std::lock_guard<std::mutex> printMutex { g_printMutex };
        std::cout
            << "\n*************************\n"
            << "Client " << m_id
            << " statistics: \ncompleted commands: " << completedCommands
            << ", \ntotal response time: " << totalTime.count() << " ms"
            << ", \nmin response time: " << minTime.count() << " ms"
            << ", \nmax response time: " << maxTime.count() << " ms"
            << ", \naverage response time: " << averageTime.count() << " ms\n"
            << "*************************\n";
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

void Client::handleResult(const std::chrono::steady_clock::time_point sentAt, const CommandResult& result)
{
    const auto receivedAt { std::chrono::steady_clock::now() };
    const auto responseTime
    {
        std::chrono::duration_cast<std::chrono::milliseconds>(receivedAt - sentAt)
    };

    {
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
    }

    /*{
        std::lock_guard<std::mutex> printMutex { g_printMutex };
        std::cout
            << "Client " << m_id
            << " received result for " << commandTypeToString(result.type)
            << " command: " << result.data
            << ", server time: " << result.executionTime.count() << " ms"
            << ", response time: " << responseTime.count() << " ms\n";
    }*/

    m_conditionVariable.notify_one();
}
