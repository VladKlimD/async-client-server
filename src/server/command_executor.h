#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class CommandExecutor
{
public:
    using Task = std::function<void()>;

    explicit CommandExecutor(std::size_t workersCount);
    ~CommandExecutor();

    CommandExecutor(const CommandExecutor&) = delete;
    CommandExecutor& operator=(const CommandExecutor&) = delete;

    // Добавление задачи в очередь выполнения
    void enqueue(Task task);

private:
    // Основной цикл worker потока
    void workerLoop();

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
