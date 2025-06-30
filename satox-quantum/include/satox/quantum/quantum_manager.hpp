#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace satox {
namespace quantum {

class QuantumManager {
public:
    QuantumManager();
    ~QuantumManager();

    // Initialize the quantum manager
    bool initialize();

    // Shutdown the quantum manager
    void shutdown();

    // Key generation
    bool generateQuantumKeyPair(std::string& publicKey, std::string& privateKey);
    bool generateQuantumSharedSecret(const std::string& publicKey, const std::string& privateKey, const std::string& serverPublicKey, std::string& sharedSecret);

    // Encryption/Decryption
    bool encrypt(const std::string& publicKey, const std::string& data, std::string& encryptedData);
    bool decrypt(const std::string& privateKey, const std::string& encryptedData, std::string& decryptedData);

    // Signing/Verification
    bool sign(const std::string& privateKey, const std::string& message, std::string& signature);
    bool verify(const std::string& publicKey, const std::string& message, const std::string& signature);

    // Hash functions
    bool computeQuantumHash(const std::string& data, std::string& hash);
    bool verifyQuantumHash(const std::string& data, const std::string& hash);

    // Key exchange
    bool performQuantumKeyExchange(const std::string& publicKey, const std::string& serverPublicKey, std::string& sessionKey);
    bool verifyQuantumKeyExchange(const std::string& sessionKey, const std::string& privateKey);

    // Random number generation
    bool generateQuantumRandomBytes(size_t length, std::vector<uint8_t>& randomBytes);
    bool generateQuantumRandomNumber(uint64_t min, uint64_t max, uint64_t& randomNumber);

    // Status and information
    bool isQuantumResistant() const;
    std::string getAlgorithm() const;
    std::string getVersion() const;
    bool isInitialized() const;

    // Base64 helpers
    std::string base64Encode(const unsigned char* data, size_t length);
    std::vector<unsigned char> base64Decode(const std::string& encoded);

private:
    std::mutex mutex_;
    bool initialized_;
    std::string algorithm_;
    std::string version_;

    // Internal helper functions
    bool initializeQuantumSystem();
    void cleanupQuantumSystem();
    bool validateKeyPair(const std::string& publicKey, const std::string& privateKey);
};

} // namespace quantum
} // namespace satox 