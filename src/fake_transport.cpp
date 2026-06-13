#include "fake_transport.h"

#include <sstream>
#include <utility>

FakeTransport::FakeTransport(ServerHandler serverHandler)
    : m_serverHandler { std::move(serverHandler) }
{}

void FakeTransport::setServerHandler(ServerHandler serverHandler)
{
    std::lock_guard<std::mutex> lock { m_mutex };
    m_serverHandler = std::move(serverHandler);
}

void FakeTransport::registerClient(const std::uint64_t clientId, ClientResponseHandler responseHandler)
{
    std::lock_guard<std::mutex> lock { m_mutex };

    if (!responseHandler)
    {
        m_clients.erase(clientId);
        return;
    }

    m_clients[clientId] = std::move(responseHandler);
}

void FakeTransport::unregisterClient(std::uint64_t clientId)
{
    std::lock_guard<std::mutex> lock { m_mutex };
    m_clients.erase(clientId);
}

bool FakeTransport::sendToServer(const std::string& request) const
{
    ServerHandler serverHandler;

    {
        std::lock_guard<std::mutex> lock { m_mutex };
        serverHandler = m_serverHandler;
    }

    if (!serverHandler)
    {
        return false;
    }

    serverHandler(request);
    return true;
}

bool FakeTransport::sendToClient(const std::string& response) const
{
    std::uint64_t clientId {};

    if (!parseClientId(response, clientId))
    {
        return false;
    }

    ClientResponseHandler responseHandler;

    {
        std::lock_guard<std::mutex> lock { m_mutex };

        const auto clientIt { m_clients.find(clientId) };
        if (clientIt == m_clients.end())
        {
            return false;
        }

        responseHandler = clientIt->second;
    }

    responseHandler(response);
    return true;
}

bool FakeTransport::parseClientId(const std::string& data, std::uint64_t& clientId)
{
    std::istringstream stream { data };
    return static_cast<bool>(stream >> clientId);
}
