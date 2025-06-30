#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

namespace satox {
namespace api {

class APIManager {
public:
    static APIManager& getInstance();

    // Prevent copying
    APIManager(const APIManager&) = delete;
    APIManager& operator=(const APIManager&) = delete;

    // Initialize the API manager
    bool initialize(const std::string& configPath);
    
    // Shutdown the API manager
    void shutdown();

    // Start all API servers
    bool start();
    
    // Stop all API servers
    void stop();

    // Register API endpoints
    bool registerEndpoint(const std::string& path, 
                         const std::string& method,
                         std::function<nlohmann::json(const nlohmann::json&)> handler);

    // Get API status
    nlohmann::json getStatus() const;

private:
    APIManager() = default;
    ~APIManager() = default;

    bool initialized_ = false;
    std::string configPath_;
    std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> endpoints_;
};

} // namespace api
} // namespace satox 