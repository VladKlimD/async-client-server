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

    void executeCommandAsync(Command command, ResultCallback callback);

private:
    void workerLoop();
    CommandResult processCommand(const Command& command);

    struct Task
    {
        Command command;
        ResultCallback callback;
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