#include <emscripten/bind.h>
#include <string>

using namespace emscripten;

class NetworkManager {
public:
    NetworkManager() : initialized(false), connected(false), port(60777) {}

    void initialize() {
        if (initialized) throw std::runtime_error("NetworkManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void connect(const std::string& host, int port) {
        if (!initialized) throw std::runtime_error("NetworkManager not initialized");
        if (connected) throw std::runtime_error("Already connected");
        if (port != 60777) throw std::runtime_error("Invalid port for Satoxcoin network");
        connected = true;
        this->port = port;
    }

    void disconnect() {
        if (!connected) throw std::runtime_error("Not connected");
        connected = false;
    }

    bool isConnected() const { return connected; }
    int getPort() const { return port; }

private:
    bool initialized;
    bool connected;
    int port;
};

EMSCRIPTEN_BINDINGS(NetworkManager) {
    class_<NetworkManager>("NetworkManager")
        .constructor<>()
        .function("initialize", &NetworkManager::initialize)
        .function("isInitialized", &NetworkManager::isInitialized)
        .function("connect", &NetworkManager::connect)
        .function("disconnect", &NetworkManager::disconnect)
        .function("isConnected", &NetworkManager::isConnected)
        .function("getPort", &NetworkManager::getPort)
        ;
} 