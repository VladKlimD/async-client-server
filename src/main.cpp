#include <cctype>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

struct ProgramOptions
{
    std::size_t clients_count = 0;
    std::size_t commands_per_client = 0;
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
        unsigned long long parsed { 0 };
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
        printUsage();
        return false;
    }

    if (!parsePositiveSize(clientsCount, options.clients_count) ||
        !parsePositiveSize(commandsPerClient, options.commands_per_client))
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
        << "Clients count: " << options.clients_count << '\n'
        << "Commands per client: " << options.commands_per_client << '\n';

    return 0;
}
