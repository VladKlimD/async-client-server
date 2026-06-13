#include "server_network.h"

#include <sstream>
#include <utility>

ServerNetwork::ServerNetwork(Server& server, ResponseHandler responseHandler)
    : m_server { server },
      m_responseHandler { std::move(responseHandler) }
{}

void ServerNetwork::receive(const std::string& data) const
{
    Request request;
    std::string error;

    if (!parse(data, request, error))
    {
        sendError(request.context, error);
        return;
    }

    handleRequest(request);
}

bool ServerNetwork::parse(const std::string& data, Request& request, std::string& error)
{
    std::istringstream stream { data };

    if (!(stream >> request.context.clientId >> request.context.requestId >> request.commandName))
    {
        error = "invalid request";
        return false;
    }

    if (request.commandName == "sin_slow" ||
        request.commandName == "cos_medium")
    {
        if (!(stream >> request.argument))
        {
            error = "missing command argument";
            return false;
        }

        std::string extra;
        if (stream >> extra)
        {
            error = "unexpected extra argument";
            return false;
        }
    }
    else if (request.commandName == "fast")
    {
        std::string extra;
        if (stream >> extra)
        {
            error = "unexpected extra argument";
            return false;
        }
    }
    else
    {
        return true;
    }

    return true;
}

void ServerNetwork::handleRequest(const Request& request) const
{
    if (request.commandName == "sin_slow")
    {
        const Server::SinSlowResultCallback callback { [this](const Server::CommandResult& result, const double value)
            { sendResponse(result, value); }};
        m_server.calculateSinSlow(request.context, request.argument, callback);

        return;
    }

    if (request.commandName == "cos_medium")
    {
        const Server::CosMediumResultCallback callback { [this](const Server::CommandResult& result, const double value)
            { sendResponse(result, value); }};
        m_server.calculateCosMedium(request.context, request.argument, callback);

        return;
    }

    if (request.commandName == "fast")
    {
        const Server::ResultCallback callback { [this](const Server::CommandResult& result) { sendResponse(result); }};
        m_server.executeFastCommand(request.context, callback);

        return;
    }

    sendError(request.context, "unknown command");
}

void ServerNetwork::sendError(const Server::CommandContext context, const std::string& error) const
{
    if (!m_responseHandler)
    {
        return;
    }

    std::ostringstream stream;
    stream
        << context.clientId << ' '
        << context.requestId << ' '
        << "error "
        << error;

    m_responseHandler(stream.str());
}

void ServerNetwork::sendResponse(const Server::CommandResult& result) const
{
    if (!m_responseHandler)
    {
        return;
    }

    std::ostringstream stream;
    stream
        << result.clientId << ' '
        << result.requestId << ' '
        << "ok "
        << result.executionTime.count();

    m_responseHandler(stream.str());
}

void ServerNetwork::sendResponse(const Server::CommandResult& result, const double value) const
{
    if (!m_responseHandler)
    {
        return;
    }

    std::ostringstream stream;
    stream
        << result.clientId << ' '
        << result.requestId << ' '
        << "ok "
        << result.executionTime.count() << ' '
        << value;

    m_responseHandler(stream.str());
}
