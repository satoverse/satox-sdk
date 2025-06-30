#include <emscripten/bind.h>
#include <string>
#include <vector>
#include <map>

using namespace emscripten;

class SecurityManager {
public:
    SecurityManager() : initialized(false), encryptionEnabled(false) {}

    void initialize() {
        if (initialized) throw std::runtime_error("SecurityManager already initialized");
        initialized = true;
    }

    bool isInitialized() const { return initialized; }

    void enableEncryption() {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        encryptionEnabled = true;
    }

    void disableEncryption() {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        encryptionEnabled = false;
    }

    bool isEncryptionEnabled() const { return encryptionEnabled; }

    std::string encrypt(const std::string& data) {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        if (!encryptionEnabled) throw std::runtime_error("Encryption not enabled");
        // TODO: Implement actual encryption
        return "encrypted_" + data;
    }

    std::string decrypt(const std::string& data) {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        if (!encryptionEnabled) throw std::runtime_error("Encryption not enabled");
        // TODO: Implement actual decryption
        return data.substr(10); // Remove "encrypted_" prefix
    }

    std::string generateKey() {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        // TODO: Implement actual key generation
        return "key_" + std::to_string(keys.size());
    }

    void addKey(const std::string& key) {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        keys.push_back(key);
    }

    void removeKey(const std::string& key) {
        if (!initialized) throw std::runtime_error("SecurityManager not initialized");
        auto it = std::find(keys.begin(), keys.end(), key);
        if (it != keys.end()) keys.erase(it);
    }

    std::vector<std::string> getKeys() const { return keys; }

private:
    bool initialized;
    bool encryptionEnabled;
    std::vector<std::string> keys;
};

EMSCRIPTEN_BINDINGS(SecurityManager) {
    class_<SecurityManager>("SecurityManager")
        .constructor<>()
        .function("initialize", &SecurityManager::initialize)
        .function("isInitialized", &SecurityManager::isInitialized)
        .function("enableEncryption", &SecurityManager::enableEncryption)
        .function("disableEncryption", &SecurityManager::disableEncryption)
        .function("isEncryptionEnabled", &SecurityManager::isEncryptionEnabled)
        .function("encrypt", &SecurityManager::encrypt)
        .function("decrypt", &SecurityManager::decrypt)
        .function("generateKey", &SecurityManager::generateKey)
        .function("addKey", &SecurityManager::addKey)
        .function("removeKey", &SecurityManager::removeKey)
        .function("getKeys", &SecurityManager::getKeys)
        ;
} 