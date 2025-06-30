#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace satox {
namespace quantum {

class HybridEncryption {
public:
    HybridEncryption();
    ~HybridEncryption();

    // Initialize the hybrid encryption system
    bool initialize();

    // Shutdown the hybrid encryption system
    void shutdown();

    // Hybrid encryption/decryption
    bool encrypt(const std::string& data, 
                const std::string& quantumPublicKey,
                const std::string& classicalPublicKey,
                std::string& encryptedData);

    bool decrypt(const std::string& encryptedData,
                const std::string& quantumPrivateKey,
                const std::string& classicalPrivateKey,
                std::string& decryptedData);

    // Key management
    bool rotateKeys(const std::string& oldQuantumKey,
                   const std::string& oldClassicalKey,
                   std::string& newQuantumKey,
                   std::string& newClassicalKey);

    bool reencrypt(const std::string& encryptedData,
                  const std::string& oldQuantumKey,
                  const std::string& oldClassicalKey,
                  const std::string& newQuantumKey,
                  const std::string& newClassicalKey,
                  std::string& reencryptedData);

    // Key storage
    bool storeKeys(const std::string& quantumKey,
                  const std::string& classicalKey,
                  const std::string& identifier);

    bool retrieveKeys(const std::string& identifier,
                     std::string& quantumKey,
                     std::string& classicalKey);

    // Status and information
    bool isInitialized() const;
    std::string getAlgorithm() const;
    std::string getVersion() const;

private:
    std::mutex mutex_;
    bool initialized_;
    std::string algorithm_;
    std::string version_;

    // Internal helper functions
    bool initializeEncryptionSystem();
    void cleanupEncryptionSystem();
    bool validateKeys(const std::string& quantumKey, const std::string& classicalKey);
    bool generateSessionKey(std::string& sessionKey);
    bool encryptWithSessionKey(const std::string& data, const std::string& sessionKey, std::string& encryptedData);
    bool decryptWithSessionKey(const std::string& encryptedData, const std::string& sessionKey, std::string& decryptedData);
    
    // Storage and encoding helpers
    std::string getStoragePath(const std::string& identifier) const;
    std::string base64Encode(const unsigned char* data, size_t length);
    std::vector<unsigned char> base64Decode(const std::string& encoded);
};

} // namespace quantum
} // namespace satox 