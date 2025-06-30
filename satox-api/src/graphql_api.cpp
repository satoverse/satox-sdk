#include "satox/api/graphql_api.hpp"
#include <crow.h>
#include <graphql-cpp/graphql.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <condition_variable>

namespace satox {
namespace api {

GraphQLAPI& GraphQLAPI::getInstance() {
    static GraphQLAPI instance;
    return instance;
}

bool GraphQLAPI::initialize(const std::string& host, int port) {
    if (initialized_) {
        return true;
    }

    try {
        host_ = host;
        port_ = port;
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing GraphQL API: " << e.what() << std::endl;
        return false;
    }
}

void GraphQLAPI::shutdown() {
    if (!initialized_) {
        return;
    }

    stop();
    resolvers_.clear();
    mutations_.clear();
    subscriptions_.clear();
    initialized_ = false;
}

bool GraphQLAPI::start() {
    if (!initialized_) {
        return false;
    }

    try {
        // Start the GraphQL server
        server_.start(host_, port_);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error starting GraphQL API: " << e.what() << std::endl;
        return false;
    }
}

void GraphQLAPI::stop() {
    if (!initialized_) {
        return;
    }

    try {
        server_.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error stopping GraphQL API: " << e.what() << std::endl;
    }
}

bool GraphQLAPI::registerResolver(const std::string& type, const std::string& field, const std::function<nlohmann::json(const nlohmann::json&)>& resolver) {
    if (!initialized_) {
        return false;
    }

    try {
        resolvers_[type + "." + field] = resolver;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering resolver: " << e.what() << std::endl;
        return false;
    }
}

bool GraphQLAPI::registerMutation(const std::string& name, const std::function<nlohmann::json(const nlohmann::json&)>& mutation) {
    if (!initialized_) {
        return false;
    }

    try {
        mutations_[name] = mutation;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering mutation: " << e.what() << std::endl;
        return false;
    }
}

bool GraphQLAPI::registerSubscription(const std::string& name, const std::function<void(const std::string&)>& subscription) {
    if (!initialized_) {
        return false;
    }

    try {
        subscriptions_[name] = subscription;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error registering subscription: " << e.what() << std::endl;
        return false;
    }
}

nlohmann::json GraphQLAPI::executeQuery(const std::string& query, const nlohmann::json& variables) {
    if (!initialized_) {
        return {{"errors", {{"message", "GraphQL API not initialized"}}}};
    }

    try {
        // Parse the query
        auto parsedQuery = graphql::parseQuery(query);
        if (!parsedQuery) {
            return {{"errors", {{"message", "Invalid query"}}}};
        }

        // Execute the query
        auto result = executeOperation(parsedQuery, variables);
        return result;
    } catch (const std::exception& e) {
        return {{"errors", {{"message", e.what()}}}};
    }
}

nlohmann::json GraphQLAPI::executeOperation(const std::shared_ptr<graphql::Operation>& operation, const nlohmann::json& variables) {
    nlohmann::json result;

    // Execute the operation based on its type
    if (operation->type == "query") {
        result = executeQueryOperation(operation, variables);
    } else if (operation->type == "mutation") {
        result = executeMutationOperation(operation, variables);
    } else if (operation->type == "subscription") {
        result = executeSubscriptionOperation(operation, variables);
    } else {
        result = {{"errors", {{"message", "Unknown operation type"}}}};
    }

    return result;
}

nlohmann::json GraphQLAPI::executeQueryOperation(const std::shared_ptr<graphql::Operation>& operation, const nlohmann::json& variables) {
    nlohmann::json result;

    // Execute each field in the query
    for (const auto& field : operation->fields) {
        auto resolver = resolvers_.find(field.type + "." + field.name);
        if (resolver != resolvers_.end()) {
            result[field.alias.empty() ? field.name : field.alias] = resolver->second(variables);
        } else {
            result[field.alias.empty() ? field.name : field.alias] = nullptr;
        }
    }

    return result;
}

nlohmann::json GraphQLAPI::executeMutationOperation(const std::shared_ptr<graphql::Operation>& operation, const nlohmann::json& variables) {
    nlohmann::json result;

    // Execute each field in the mutation
    for (const auto& field : operation->fields) {
        auto mutation = mutations_.find(field.name);
        if (mutation != mutations_.end()) {
            result[field.alias.empty() ? field.name : field.alias] = mutation->second(variables);
        } else {
            result[field.alias.empty() ? field.name : field.alias] = nullptr;
        }
    }

    return result;
}

nlohmann::json GraphQLAPI::executeSubscriptionOperation(const std::shared_ptr<graphql::Operation>& operation, const nlohmann::json& variables) {
    nlohmann::json result;

    // Execute each field in the subscription
    for (const auto& field : operation->fields) {
        auto subscription = subscriptions_.find(field.name);
        if (subscription != subscriptions_.end()) {
            // Start the subscription
            subscription->second(variables.dump());
            result[field.alias.empty() ? field.name : field.alias] = {{"status", "subscribed"}};
        } else {
            result[field.alias.empty() ? field.name : field.alias] = nullptr;
        }
    }

    return result;
}

bool GraphQLAPI::isInitialized() const {
    return initialized_;
}

std::string GraphQLAPI::getHost() const {
    return host_;
}

int GraphQLAPI::getPort() const {
    return port_;
}

nlohmann::json GraphQLAPI::getStatus() const {
    nlohmann::json status;
    status["initialized"] = initialized_;
    status["host"] = host_;
    status["port"] = port_;
    
    status["resolvers"] = nlohmann::json::array();
    for (const auto& resolver : resolvers_) {
        status["resolvers"].push_back(resolver.first);
    }

    status["mutations"] = nlohmann::json::array();
    for (const auto& mutation : mutations_) {
        status["mutations"].push_back(mutation.first);
    }

    status["subscriptions"] = nlohmann::json::array();
    for (const auto& subscription : subscriptions_) {
        status["subscriptions"].push_back(subscription.first);
    }

    return status;
}

} // namespace api
} // namespace satox 