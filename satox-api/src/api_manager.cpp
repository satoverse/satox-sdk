#include "satox/api/api_manager.hpp"
#include "satox/api/rest_api.hpp"
#include "satox/api/websocket_api.hpp"
#include "satox/api/graphql_api.hpp"
#include <fstream>
#include <iostream>

namespace satox {
namespace api {

APIManager& APIManager::getInstance() {
    static APIManager instance;
    return instance;
}

bool APIManager::initialize(const std::string& configPath) {
    if (initialized_) {
        return true;
    }

    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            std::cerr << "Failed to open config file: " << configPath << std::endl;
            return false;
        }

        nlohmann::json config;
        configFile >> config;

        // Initialize REST API
        auto& restApi = RESTAPI::getInstance();
        if (!restApi.initialize(config["rest"]["host"], config["rest"]["port"])) {
            std::cerr << "Failed to initialize REST API" << std::endl;
            return false;
        }

        // Initialize WebSocket API
        auto& wsApi = WebSocketAPI::getInstance();
        if (!wsApi.initialize(config["websocket"]["host"], config["websocket"]["port"])) {
            std::cerr << "Failed to initialize WebSocket API" << std::endl;
            return false;
        }

        // Initialize GraphQL API
        auto& graphqlApi = GraphQLAPI::getInstance();
        if (!graphqlApi.initialize(config["graphql"]["host"], config["graphql"]["port"])) {
            std::cerr << "Failed to initialize GraphQL API" << std::endl;
            return false;
        }

        configPath_ = configPath;
        initialized_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing API Manager: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::shutdown() {
    if (!initialized_) {
        return true;
    }

    try {
        // Shutdown REST API
        auto& restApi = RESTAPI::getInstance();
        restApi.shutdown();

        // Shutdown WebSocket API
        auto& wsApi = WebSocketAPI::getInstance();
        wsApi.shutdown();

        // Shutdown GraphQL API
        auto& graphqlApi = GraphQLAPI::getInstance();
        graphqlApi.shutdown();

        initialized_ = false;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error shutting down API Manager: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::isInitialized() const {
    return initialized_;
}

std::string APIManager::getConfigPath() const {
    return configPath_;
}

bool APIManager::registerEndpoint(const std::string& path, const std::string& method, const std::function<void(const crow::request&, crow::response&)>& handler) {
    if (!initialized_) {
        return false;
    }

    try {
        auto& restApi = RESTAPI::getInstance();
        return restApi.registerEndpoint(path, method, handler);
    } catch (const std::exception& e) {
        std::cerr << "Error registering endpoint: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::registerWebSocketHandler(const std::string& path, const std::function<void(const std::string&)>& handler) {
    if (!initialized_) {
        return false;
    }

    try {
        auto& wsApi = WebSocketAPI::getInstance();
        return wsApi.registerHandler(path, handler);
    } catch (const std::exception& e) {
        std::cerr << "Error registering WebSocket handler: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::registerGraphQLResolver(const std::string& type, const std::string& field, const std::function<nlohmann::json(const nlohmann::json&)>& resolver) {
    if (!initialized_) {
        return false;
    }

    try {
        auto& graphqlApi = GraphQLAPI::getInstance();
        return graphqlApi.registerResolver(type, field, resolver);
    } catch (const std::exception& e) {
        std::cerr << "Error registering GraphQL resolver: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::registerGraphQLMutation(const std::string& name, const std::function<nlohmann::json(const nlohmann::json&)>& mutation) {
    if (!initialized_) {
        return false;
    }

    try {
        auto& graphqlApi = GraphQLAPI::getInstance();
        return graphqlApi.registerMutation(name, mutation);
    } catch (const std::exception& e) {
        std::cerr << "Error registering GraphQL mutation: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::registerGraphQLSubscription(const std::string& name, const std::function<void(const std::string&)>& subscription) {
    if (!initialized_) {
        return false;
    }

    try {
        auto& graphqlApi = GraphQLAPI::getInstance();
        return graphqlApi.registerSubscription(name, subscription);
    } catch (const std::exception& e) {
        std::cerr << "Error registering GraphQL subscription: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::start() {
    if (!initialized_) {
        return false;
    }

    try {
        // Start REST API
        auto& restApi = RESTAPI::getInstance();
        if (!restApi.start()) {
            return false;
        }

        // Start WebSocket API
        auto& wsApi = WebSocketAPI::getInstance();
        if (!wsApi.start()) {
            restApi.stop();
            return false;
        }

        // Start GraphQL API
        auto& graphqlApi = GraphQLAPI::getInstance();
        if (!graphqlApi.start()) {
            restApi.stop();
            wsApi.stop();
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error starting API Manager: " << e.what() << std::endl;
        return false;
    }
}

bool APIManager::stop() {
    if (!initialized_) {
        return true;
    }

    try {
        // Stop REST API
        auto& restApi = RESTAPI::getInstance();
        restApi.stop();

        // Stop WebSocket API
        auto& wsApi = WebSocketAPI::getInstance();
        wsApi.stop();

        // Stop GraphQL API
        auto& graphqlApi = GraphQLAPI::getInstance();
        graphqlApi.stop();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping API Manager: " << e.what() << std::endl;
        return false;
    }
}

nlohmann::json APIManager::getStatus() const {
    nlohmann::json status;
    status["initialized"] = initialized_;
    status["config_path"] = configPath_;
    status["endpoints"] = nlohmann::json::array();
    
    for (const auto& endpoint : endpoints_) {
        status["endpoints"].push_back(endpoint.first);
    }

    status["rest"] = RESTAPI::getInstance().getStatus();
    status["websocket"] = WebSocketAPI::getInstance().getStatus();
    status["graphql"] = GraphQLAPI::getInstance().getStatus();

    return status;
}

} // namespace api
} // namespace satox 