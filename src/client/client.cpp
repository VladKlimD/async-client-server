#include "client.h"
#include "fake_transport.h"

#include <iostream>
#include <mutex>
#include <utility>

namespace { std::mutex g_printMutex; }

Client::Client(const std::uint64_t id, const std::size_t commandsCount, FakeTransport& transport)
    : m_id { id }, m_commandsCount { commandsCount }, m_network { id, transport }
{}

void Client::run()
{
    const auto startedAt { std::chrono::steady_clock::now() };

    for (std::size_t i = 0; i < m_commandsCount; ++i)
    {
        sendCommand(i);
    }

    {
        std::unique_lock<std::mutex> lock { m_mutex };
        m_conditionVariable.wait(lock, [this]()
        {
            return m_completedCommands == m_commandsCount;
        });
    }

    printStatistics(startedAt);
}

void Client::sendCommand(const std::size_t index)
{
    const double angleRadians { static_cast<double>(index + 1) * 0.1 };
    const auto sentAt { std::chrono::steady_clock::now() };

    std::string commandName;
    std::uint64_t requestId {};

    switch (index % 3)
    {
        case 0:
        {
            commandName = "fast";
            const ClientNetwork::ResultCallback resultCallback =
            [this, sentAt, commandName](const ClientNetwork::CommandResult& result)
            {
                handleResult(sentAt, commandName, result);
            };

            requestId = m_network.executeFastCommand(resultCallback);

            break;
        }

        case 1:
        {
            commandName = "cos_medium";
            const ClientNetwork::CosMediumResultCallback resultCallback =
            [this, sentAt, commandName](const ClientNetwork::CommandResult& result, double)
            {
                handleResult(sentAt, commandName, result);
            };

            requestId = m_network.calculateCosMedium(angleRadians, resultCallback);

            break;
        }

        default:
        {
            commandName = "sin_slow";
            const ClientNetwork::SinSlowResultCallback resultCallback =
            [this, sentAt, commandName](const ClientNetwork::CommandResult& result, double)
            {
                handleResult(sentAt, commandName, result);
            };

            requestId = m_network.calculateSinSlow(angleRadians, resultCallback);

            break;
        }
    }

    {
        std::lock_guard<std::mutex> printMutex { g_printMutex };
        std::cout
            << "Client " << m_id
            << " sent " << commandName
            << " request " << requestId;

        if (commandName != "fast")
        {
            std::cout << ", argument: " << angleRadians;
        }

        std::cout << '\n';
    }
}

void Client::handleResult(const std::chrono::steady_clock::time_point sentAt, const std::string& commandName,
                          const ClientNetwork::CommandResult& result)
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
            m_minResponseTime = responseTime;
            m_maxResponseTime = responseTime;
        }
        else
        {
            if (responseTime < m_minResponseTime)
            {
                m_minResponseTime = responseTime;
            }

            if (responseTime > m_maxResponseTime)
            {
                m_maxResponseTime = responseTime;
            }
        }

        m_totalResponseTime += responseTime;
        m_totalServerExecutionTime += result.executionTime;
        ++m_completedCommands;
    }

    if (!result.success)
    {
        std::lock_guard<std::mutex> printMutex { g_printMutex };
        std::cout
            << "Client " << m_id
            << " received error for " << commandName
            << " request " << result.requestId
            << ": " << result.error << '\n';
    }

    m_conditionVariable.notify_one();
}

void Client::printStatistics(const std::chrono::steady_clock::time_point startedAt)
{
    std::unique_lock<std::mutex> lock { m_mutex };
    const std::size_t completedCommands { m_completedCommands };
    const auto totalResponseTime { m_totalResponseTime };
    const auto minResponseTime { m_minResponseTime };
    const auto maxResponseTime { m_maxResponseTime };
    const auto totalServerExecutionTime { m_totalServerExecutionTime };
    lock.unlock();

    const auto finishedAt { std::chrono::steady_clock::now() };
    const auto clientElapsedTime
    {
        std::chrono::duration_cast<std::chrono::milliseconds>(finishedAt - startedAt)
    };
    const auto completedCommandsCount
    {
        static_cast<std::chrono::milliseconds::rep>(completedCommands)
    };
    const auto averageResponseTime
    {
        completedCommands == 0
            ? std::chrono::milliseconds { 0 }
            : std::chrono::milliseconds { totalResponseTime.count() / completedCommandsCount }
    };
    const auto averageServerExecutionTime
    {
        completedCommands == 0
            ? std::chrono::milliseconds { 0 }
            : std::chrono::milliseconds { totalServerExecutionTime.count() / completedCommandsCount }
    };

    std::lock_guard<std::mutex> printMutex { g_printMutex };
    std::cout
        << "\n*************************\n"
        << "Client " << m_id
        << " statistics:\n"
        << "completed commands: " << completedCommands << '\n'
        << "client elapsed time: " << clientElapsedTime.count() << " ms\n"
        << "response time total: " << totalResponseTime.count() << " ms\n"
        << "response time min: " << minResponseTime.count() << " ms\n"
        << "response time max: " << maxResponseTime.count() << " ms\n"
        << "response time average: " << averageResponseTime.count() << " ms\n"
        << "server execution time total: " << totalServerExecutionTime.count() << " ms\n"
        << "server execution time average: " << averageServerExecutionTime.count() << " ms\n"
        << "*************************\n";
}
