#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace api {

class WebSocketAPI {
public:
    static WebSocketAPI& getInstance();

    // Prevent copying
    WebSocketAPI(const WebSocketAPI&) = delete;
    WebSocketAPI& operator=(const WebSocketAPI&) = delete;

    // Initialize the WebSocket server
    bool initialize(const std::string& host, int port);
    
    // Shutdown the WebSocket server
    void shutdown();

    // Start the WebSocket server
    bool start();
    
    // Stop the WebSocket server
    void stop();

    // Register a WebSocket endpoint
    bool registerEndpoint(const std::string& path,
                         std::function<void(const nlohmann::json&, std::function<void(const nlohmann::json&)>)> handler);

    // Broadcast message to all connected clients
    bool broadcast(const std::string& path, const nlohmann::json& message);

    // Get server status
    nlohmann::json getStatus() const;

private:
    WebSocketAPI() = default;
    ~WebSocketAPI() = default;

    bool initialized_ = false;
    std::string host_;
    int port_;
    std::unordered_map<std::string, std::function<void(const nlohmann::json&, std::function<void(const nlohmann::json&)>)>> endpoints_;
};

} // namespace api
} // namespace satox 