#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <chrono>

namespace satox {
namespace quantum {

class KeyStorage {
public:
    KeyStorage();
    ~KeyStorage();

    // Initialize the key storage system
    bool initialize();

    // Shutdown the key storage system
    void shutdown();

    // Key storage operations
    bool storeKey(const std::string& identifier,
                 const std::string& key,
                 const std::string& metadata = std::string());

    bool retrieveKey(const std::string& identifier,
                    std::string& key,
                    std::string& metadata);

    bool deleteKey(const std::string& identifier);

    bool updateKey(const std::string& key,
                  const std::string& identifier,
                  const std::string& metadata = "");

    // Key metadata operations
    bool setKeyMetadata(const std::string& identifier,
                       const std::string& metadata);

    bool getKeyMetadata(const std::string& identifier,
                       std::string& metadata);

    // Key rotation operations
    bool rotateKey(const std::string& identifier,
                  const std::string& newKey,
                  const std::string& metadata = "");

    bool reencryptKey(const std::string& identifier,
                     const std::string& oldKey,
                     const std::string& newKey);

    // Key validation
    bool validateKey(const std::string& key,
                    const std::string& identifier);

    bool validateAllKeys();

    // Key expiration
    bool setKeyExpiration(const std::string& identifier,
                         const std::chrono::system_clock::time_point& expiration);

    bool getKeyExpiration(const std::string& identifier,
                         std::chrono::system_clock::time_point& expiration);

    bool isKeyExpired(const std::string& identifier);

    // Key access control
    bool setKeyAccess(const std::string& identifier,
                     const std::vector<std::string>& allowedUsers);

    bool getKeyAccess(const std::string& identifier,
                     std::vector<std::string>& allowedUsers);

    bool checkKeyAccess(const std::string& identifier,
                       const std::string& user);

    // Status and information
    bool isInitialized() const;
    std::string getAlgorithm() const;
    std::string getVersion() const;
    size_t getKeyCount() const;
    std::vector<std::string> getAllKeyIdentifiers() const;

private:
    struct KeyEntry {
        std::string key;
        std::string metadata;
        std::chrono::system_clock::time_point expiration;
        std::vector<std::string> allowedUsers;
        std::chrono::system_clock::time_point lastAccess;
        size_t accessCount;
    };

    mutable std::mutex mutex_;
    bool initialized_;
    std::string algorithm_;
    std::string version_;
    std::unordered_map<std::string, KeyEntry> keys_;

    // Internal helper functions
    bool initializeStorageSystem();
    void cleanupStorageSystem();
    bool encryptKey(const std::string& key, std::string& encryptedKey);
    bool decryptKey(const std::string& encryptedKey, std::string& key);
    bool validateKeyFormat(const std::string& key);
    bool validateKeyMetadata(const std::string& metadata);
    bool validateKeyAccess(const KeyEntry& entry, const std::string& user);
    void updateKeyAccess(KeyEntry& entry);
    bool cleanupExpiredKeys();
};

} // namespace quantum
} // namespace satox 