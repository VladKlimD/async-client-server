#include <iostream>
#include <limits>
#include <sstream>
#include <string>

struct ProgramOptions
{
    size_t clientsCount { 0 };
    size_t commandsPerClient { 0 };
};

bool parseProgramOptions(int argc, char* argv[], ProgramOptions& options)
{
    const auto printUsage = [argv]()
    {
        std::cout
            << "Usage:\n"
            << "  " << argv[0] << " <clients_count> <commands_per_client>\n"
            << "  " << argv[0] << "\n";
    };

    const auto parsePositiveSize = [](const std::string& text, size_t& value)
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
        size_t parsed { 0 };
        stream >> parsed;

        if (!stream || parsed == 0 || parsed > std::numeric_limits<size_t>::max())
        {
            return false;
        }

        value = static_cast<size_t>(parsed);
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
        printUsage();
        return false;
    }

    if (!parsePositiveSize(clientsCount, options.clientsCount) ||
        !parsePositiveSize(commandsPerClient, options.commandsPerClient))
    {
        printUsage();
        return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    ProgramOptions options;

    if (!parseProgramOptions(argc, argv, options))
    {
        return 1;
    }

    std::cout
        << "Clients count: " << options.clientsCount << '\n'
        << "Commands per client: " << options.commandsPerClient << '\n';

    return 0;
}
