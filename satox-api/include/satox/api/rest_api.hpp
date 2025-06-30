#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace api {

class RESTAPI {
public:
    static RESTAPI& getInstance();

    // Prevent copying
    RESTAPI(const RESTAPI&) = delete;
    RESTAPI& operator=(const RESTAPI&) = delete;

    // Initialize the REST API server
    bool initialize(const std::string& host, int port);
    
    // Shutdown the REST API server
    void shutdown();

    // Start the REST API server
    bool start();
    
    // Stop the REST API server
    void stop();

    // Register a REST endpoint
    bool registerEndpoint(const std::string& path,
                         const std::string& method,
                         std::function<nlohmann::json(const nlohmann::json&)> handler);

    // Get server status
    nlohmann::json getStatus() const;

private:
    RESTAPI() = default;
    ~RESTAPI() = default;

    bool initialized_ = false;
    std::string host_;
    int port_;
    std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> endpoints_;
};

} // namespace api
} // namespace satox 