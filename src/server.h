#pragma once

#include "command.h"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>

class Server
{
public:
    explicit Server(std::size_t workersCount);
    ~Server();

    void executeCommand(Command command, ResultCallback callback);

private:
    void workerLoop();
    CommandResult processCommand(const Command& command);

    struct Task
    {
        Command command;
        ResultCallback callback;
    };

    std::queue<Task> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_conditionVariable;
    std::vector<std::thread> m_workers;
    bool m_stopped { false };
};