#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <limits>

struct ProgramOptions
{
    std::size_t clientsCount { 0 };
    std::size_t commandsPerClient { 0 };
};

inline bool parseProgramOptions(int argc, char* argv[], ProgramOptions& options)
{
    const auto parsePositiveSize = [](const std::string& text, std::size_t& value)
    {
        if (text.empty())
            return false;

        for (const char ch : text)
        {
            if (!std::isdigit(static_cast<unsigned char>(ch)))
            {
                return false;
            }
        }

        std::istringstream stream { text };
        std::size_t parsed { 0 };
        stream >> parsed;

        if (!stream || parsed == 0 || parsed > std::numeric_limits<std::size_t>::max())
        {
            return false;
        }

        value = static_cast<std::size_t>(parsed);
        return true;
    };

    std::string clientsCount;
    std::string commandsPerClient;

    if (argc == 1)
    {
        std::cout << "Enter clients count: ";
        std::cin >> clientsCount;

        std::cout << "Enter commands per client: ";
        std::cin >> commandsPerClient;
    }
    else if (argc == 3)
    {
        clientsCount = argv[1];
        commandsPerClient = argv[2];
    }
    else
    {
        return false;
    }

    if (!parsePositiveSize(clientsCount, options.clientsCount) ||
        !parsePositiveSize(commandsPerClient, options.commandsPerClient))
    {
        return false;
    }

    return true;
}