#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace satox {
namespace api {

class GraphQLAPI {
public:
    static GraphQLAPI& getInstance();

    // Prevent copying
    GraphQLAPI(const GraphQLAPI&) = delete;
    GraphQLAPI& operator=(const GraphQLAPI&) = delete;

    // Initialize the GraphQL server
    bool initialize(const std::string& host, int port);
    
    // Shutdown the GraphQL server
    void shutdown();

    // Start the GraphQL server
    bool start();
    
    // Stop the GraphQL server
    void stop();

    // Register a GraphQL resolver
    bool registerResolver(const std::string& type,
                         const std::string& field,
                         std::function<nlohmann::json(const nlohmann::json&)> resolver);

    // Register a GraphQL mutation
    bool registerMutation(const std::string& name,
                         std::function<nlohmann::json(const nlohmann::json&)> handler);

    // Register a GraphQL subscription
    bool registerSubscription(const std::string& name,
                            std::function<void(const nlohmann::json&, std::function<void(const nlohmann::json&)>)> handler);

    // Get server status
    nlohmann::json getStatus() const;

private:
    GraphQLAPI() = default;
    ~GraphQLAPI() = default;

    bool initialized_ = false;
    std::string host_;
    int port_;
    std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> resolvers_;
    std::unordered_map<std::string, std::function<nlohmann::json(const nlohmann::json&)>> mutations_;
    std::unordered_map<std::string, std::function<void(const nlohmann::json&, std::function<void(const nlohmann::json&)>)>> subscriptions_;
};

} // namespace api
} // namespace satox 