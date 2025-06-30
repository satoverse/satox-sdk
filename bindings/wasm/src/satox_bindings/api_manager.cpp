#include <emscripten/bind.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

using namespace emscripten;

class APIManager {
public:
    APIManager() : initialized(false), serverRunning(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("APIManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void startServer(int port) {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (serverRunning) throw std::runtime_error("Server already running");
        // TODO: Implement actual server start
        serverRunning = true;
    }

    void stopServer() {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (!serverRunning) throw std::runtime_error("Server not running");
        // TODO: Implement actual server stop
        serverRunning = false;
    }

    bool isServerRunning() const { return serverRunning; }

    void registerEndpoint(const std::string& path, const std::string& method) {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (!serverRunning) throw std::runtime_error("Server not running");
        endpoints[path] = method;
    }

    void unregisterEndpoint(const std::string& path) {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (!serverRunning) throw std::runtime_error("Server not running");
        if (endpoints.find(path) == endpoints.end()) throw std::runtime_error("Endpoint not found");
        endpoints.erase(path);
    }

    std::vector<std::string> listEndpoints() const {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (!serverRunning) throw std::runtime_error("Server not running");
        std::vector<std::string> result;
        for (const auto& endpoint : endpoints) {
            result.push_back(endpoint.first + " (" + endpoint.second + ")");
        }
        return result;
    }

    void setConfig(const std::string& key, const std::string& value) {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        config[key] = value;
    }

    std::string getConfig(const std::string& key) const {
        if (!initialized) throw std::runtime_error("APIManager not initialized");
        if (config.find(key) == config.end()) throw std::runtime_error("Config key not found");
        return config.at(key);
    }

private:
    bool initialized;
    bool serverRunning;
    std::map<std::string, std::string> endpoints;
    std::map<std::string, std::string> config;
};

EMSCRIPTEN_BINDINGS(APIManager) {
    class_<APIManager>("APIManager")
        .constructor<>()
        .function("initialize", &APIManager::initialize)
        .function("isInitialized", &APIManager::isInitialized)
        .function("startServer", &APIManager::startServer)
        .function("stopServer", &APIManager::stopServer)
        .function("isServerRunning", &APIManager::isServerRunning)
        .function("registerEndpoint", &APIManager::registerEndpoint)
        .function("unregisterEndpoint", &APIManager::unregisterEndpoint)
        .function("listEndpoints", &APIManager::listEndpoints)
        .function("setConfig", &APIManager::setConfig)
        .function("getConfig", &APIManager::getConfig)
        ;
} 