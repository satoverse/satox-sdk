#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <map>

using namespace emscripten;

class IPFSManager {
public:
    IPFSManager() : initialized(false), connected(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("IPFSManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void connect(const std::string& host, int port) {
        if (!initialized) throw std::runtime_error("IPFSManager not initialized");
        if (connected) throw std::runtime_error("Already connected to IPFS");
        connected = true;
    }

    void disconnect() {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        connected = false;
    }

    bool isConnected() const { return connected; }

    std::string uploadFile(const std::string& filePath) {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        // In WASM, we'll return a mock CID
        return "Qm" + std::to_string(rand());
    }

    std::vector<uint8_t> downloadFile(const std::string& cid) {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        // In WASM, we'll return mock data
        return std::vector<uint8_t>({1, 2, 3, 4, 5});
    }

    void pinFile(const std::string& cid) {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        pinnedFiles.insert(cid);
    }

    void unpinFile(const std::string& cid) {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        pinnedFiles.erase(cid);
    }

    std::vector<std::string> getPinnedFiles() const {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        return std::vector<std::string>(pinnedFiles.begin(), pinnedFiles.end());
    }

    std::map<std::string, std::string> getFileInfo(const std::string& cid) const {
        if (!connected) throw std::runtime_error("Not connected to IPFS");
        return {
            {"cid", cid},
            {"size", "1024"},
            {"type", "file"}
        };
    }

private:
    bool initialized;
    bool connected;
    std::set<std::string> pinnedFiles;
};

EMSCRIPTEN_BINDINGS(IPFSManager) {
    class_<IPFSManager>("IPFSManager")
        .constructor<>()
        .function("initialize", &IPFSManager::initialize)
        .function("isInitialized", &IPFSManager::isInitialized)
        .function("connect", &IPFSManager::connect)
        .function("disconnect", &IPFSManager::disconnect)
        .function("isConnected", &IPFSManager::isConnected)
        .function("uploadFile", &IPFSManager::uploadFile)
        .function("downloadFile", &IPFSManager::downloadFile)
        .function("pinFile", &IPFSManager::pinFile)
        .function("unpinFile", &IPFSManager::unpinFile)
        .function("getPinnedFiles", &IPFSManager::getPinnedFiles)
        .function("getFileInfo", &IPFSManager::getFileInfo)
        ;
} 