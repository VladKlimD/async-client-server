#pragma once

#include "command.h"

#include <functional>
#include <condition_variable>
#include <mutex>

// Эмулятор реального клиента
class Client
{
public:
    Client(std::size_t id, std::size_t commandsCount, CommandHandler commandHandler);

    void run();

private:
    Command makeCommand(std::size_t index) const;
    void handleResult(std::chrono::steady_clock::time_point sentAt, const CommandResult& result);

    std::size_t m_id;
    std::size_t m_commandsCount;
    CommandHandler m_commandHandler;

    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::size_t m_completedCommands { 0 };

    std::chrono::milliseconds m_totalResponseTime { 0 };
    std::chrono::milliseconds m_minResponseTime { 0 };
    std::chrono::milliseconds m_maxResponseTime { 0 };
    std::chrono::milliseconds m_totalServerExecutionTime { 0 };
};
