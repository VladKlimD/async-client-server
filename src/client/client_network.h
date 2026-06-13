#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

class FakeTransport;

class ClientNetwork
{
public:
    struct CommandResult
    {
        std::uint64_t clientId {};
        std::uint64_t requestId {};
        std::chrono::milliseconds executionTime {};
        bool success { false };
        std::string error;
    };

    using ResultCallback = std::function<void(CommandResult)>;
    using SinSlowResultCallback = std::function<void(CommandResult, double)>;
    using CosMediumResultCallback = std::function<void(CommandResult, double)>;

    ClientNetwork(std::uint64_t clientId, FakeTransport& transport);
    ~ClientNetwork();

    std::uint64_t executeFastCommand(ResultCallback callback);
    std::uint64_t calculateSinSlow(double angleRadians, SinSlowResultCallback callback);
    std::uint64_t calculateCosMedium(double angleRadians, CosMediumResultCallback callback);

private:
    struct Request
    {
        std::uint64_t clientId {};
        std::uint64_t requestId {};
        std::string commandName;
        std::string argument;
    };

    struct Response
    {
        CommandResult result;
        bool hasValue { false };
        double value {};
    };

    using ResponseCallback = std::function<void(Response)>;

    std::uint64_t sendRequest(const std::string& commandName, const std::string& argument, ResponseCallback callback);
    void receiveResponse(const std::string& data);
    void completeRequest(std::uint64_t requestId, Response response);
    void failRequest(std::uint64_t requestId, const std::string& error);

    static std::string serializeRequest(const Request& request);
    static std::string makeDoubleArgument(double value);
    static bool parseResponse(const std::string& data, Response& response);

    std::uint64_t m_clientId {};
    FakeTransport& m_transport;

    mutable std::mutex m_mutex;
    std::uint64_t m_nextRequestId { 1 };
    std::unordered_map<std::uint64_t, ResponseCallback> m_pendingRequests;
};
