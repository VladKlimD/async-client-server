#include "client_network.h"
#include "fake_transport.h"

#include <sstream>
#include <utility>

ClientNetwork::ClientNetwork(const std::uint64_t clientId, FakeTransport& transport)
    : m_clientId { clientId }, m_transport { transport }
{
    const FakeTransport::ClientResponseHandler clientResponseHandler { [this](const std::string& response)
        { receiveResponse(response); }};

    m_transport.registerClient(m_clientId, clientResponseHandler);
}

ClientNetwork::~ClientNetwork()
{
    m_transport.unregisterClient(m_clientId);
}

std::uint64_t ClientNetwork::executeFastCommand(ResultCallback callback)
{
    const ResponseCallback responseCallback { [callback = std::move(callback)](Response response)
    {
        if (callback)
        {
            callback(std::move(response.result));
        }
    }};

    return sendRequest("fast", std::string {}, responseCallback);
}

std::uint64_t ClientNetwork::calculateSinSlow(const double angleRadians, SinSlowResultCallback callback)
{
    const ResponseCallback responseCallback { [callback = std::move(callback)](Response response)
    {
        if (response.result.success && !response.hasValue)
        {
            response.result.success = false;
            response.result.error = "missing result value";
        }

        if (callback)
        {
            callback(std::move(response.result), response.value);
        }
    }};

    return sendRequest("sin_slow", makeDoubleArgument(angleRadians), responseCallback);
}

std::uint64_t ClientNetwork::calculateCosMedium(const double angleRadians, CosMediumResultCallback callback)
{
    const ResponseCallback responseCallback { [callback = std::move(callback)](Response response)
    {
        if (response.result.success && !response.hasValue)
        {
            response.result.success = false;
            response.result.error = "missing result value";
        }

        if (callback)
        {
            callback(std::move(response.result), response.value);
        }
    }};

    return sendRequest("cos_medium", makeDoubleArgument(angleRadians), responseCallback);
}

std::uint64_t ClientNetwork::sendRequest(const std::string& commandName, const std::string& argument,
    ResponseCallback callback)
{
    std::uint64_t requestId {};

    {
        std::lock_guard<std::mutex> lock { m_mutex };
        requestId = m_nextRequestId++;
        m_pendingRequests[requestId] = std::move(callback);
    }

    const Request request
    {
        m_clientId,
        requestId,
        commandName,
        argument
    };

    if (!m_transport.sendToServer(serializeRequest(request)))
    {
        failRequest(requestId, "transport error");
    }

    return requestId;
}

void ClientNetwork::receiveResponse(const std::string& data)
{
    Response response;

    if (!parseResponse(data, response) ||
        response.result.clientId != m_clientId)
    {
        return;
    }

    completeRequest(response.result.requestId, std::move(response));
}

void ClientNetwork::completeRequest(const std::uint64_t requestId, Response response)
{
    ResponseCallback callback;

    {
        std::lock_guard<std::mutex> lock { m_mutex };

        const auto requestIt { m_pendingRequests.find(requestId) };
        if (requestIt == m_pendingRequests.end())
        {
            return;
        }

        callback = std::move(requestIt->second);
        m_pendingRequests.erase(requestIt);
    }

    if (callback)
    {
        callback(std::move(response));
    }
}

void ClientNetwork::failRequest(const std::uint64_t requestId, const std::string& error)
{
    Response response;
    response.result.clientId = m_clientId;
    response.result.requestId = requestId;
    response.result.success = false;
    response.result.error = error;

    completeRequest(requestId, std::move(response));
}

std::string ClientNetwork::serializeRequest(const Request& request)
{
    std::ostringstream stream;
    stream
        << request.clientId << ' '
        << request.requestId << ' '
        << request.commandName;

    if (!request.argument.empty())
    {
        stream << ' ' << request.argument;
    }

    return stream.str();
}

std::string ClientNetwork::makeDoubleArgument(const double value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

bool ClientNetwork::parseResponse(const std::string& data, Response& response)
{
    std::istringstream stream { data };
    std::string status;

    if (!(stream >> response.result.clientId >> response.result.requestId >> status))
    {
        return false;
    }

    if (status == "ok")
    {
        long long executionTimeMs {};

        if (!(stream >> executionTimeMs))
        {
            return false;
        }

        response.result.success = true;
        response.result.executionTime = std::chrono::milliseconds { executionTimeMs };

        if (stream >> response.value)
        {
            response.hasValue = true;
        }

        return true;
    }

    if (status == "error")
    {
        response.result.success = false;
        std::getline(stream, response.result.error);

        if (!response.result.error.empty() &&
            response.result.error.front() == ' ')
        {
            response.result.error.erase(response.result.error.begin());
        }

        return true;
    }

    return false;
}
