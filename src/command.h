#pragma once

#include <string>
#include <functional>
#include <chrono>

enum class CommandType
{
    Fast,
    Medium,
    Slow
};

struct Command
{
    CommandType type;
    std::string data;
};

inline std::string commandTypeToString(const CommandType type)
{
    switch (type)
    {
        case CommandType::Fast:
            return "Fast";
        case CommandType::Medium:
            return "Medium";
        case CommandType::Slow:
            return "Slow";
    }

    return "Unknown";
}

struct CommandResult
{
    CommandType type;
    std::string data;
    std::chrono::milliseconds executionTime;
};

using ResultCallback = std::function<void(CommandResult)>;
using CommandHandler = std::function<void(Command, ResultCallback)>;
