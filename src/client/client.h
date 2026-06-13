#pragma once

#include "client_network.h"

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>

class FakeTransport;

class Client
{
public:
    Client(std::uint64_t id, std::size_t commandsCount, FakeTransport& transport);

    void run();

private:
    void sendCommand(std::size_t index);
    void handleResult(std::chrono::steady_clock::time_point sentAt, const std::string& commandName,
                      const ClientNetwork::CommandResult& result);
    void printStatistics(std::chrono::steady_clock::time_point startedAt);

    std::uint64_t m_id {};
    std::size_t m_commandsCount {};
    ClientNetwork m_network;

    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::size_t m_completedCommands { 0 };

    std::chrono::milliseconds m_totalResponseTime { 0 };
    std::chrono::milliseconds m_minResponseTime { 0 };
    std::chrono::milliseconds m_maxResponseTime { 0 };
    std::chrono::milliseconds m_totalServerExecutionTime { 0 };
};
