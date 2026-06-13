#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

class FakeTransport
{
public:
    using ServerHandler = std::function<void(std::string)>;
    using ClientResponseHandler = std::function<void(std::string)>;

    FakeTransport() = default;
    explicit FakeTransport(ServerHandler serverHandler);

    // Установка обработчика запросов сервера
    void setServerHandler(ServerHandler serverHandler);
    // Регистрация обработчика ответов клиента
    void registerClient(std::uint64_t clientId, ClientResponseHandler responseHandler);
    void unregisterClient(std::uint64_t clientId);

    bool sendToServer(const std::string& request) const;
    bool sendToClient(const std::string& response) const;

private:
    // Извлекает clientId из строкового пакета
    static bool parseClientId(const std::string& data, std::uint64_t& clientId);

    ServerHandler m_serverHandler;
    mutable std::mutex m_mutex;
    std::unordered_map<std::uint64_t, ClientResponseHandler> m_clients;
};
