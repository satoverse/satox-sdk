#pragma once

#include <satox/quantum/quantum_manager.hpp>
#include <satox/quantum/hybrid_encryption.hpp>
#include <satox/quantum/key_storage.hpp>
#include <satox/quantum/post_quantum_algorithms.hpp>
#include <satox/security_manager.hpp>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

namespace satox {
namespace quantum {

class QuantumSecurityManager {
public:
    QuantumSecurityManager();
    ~QuantumSecurityManager();

    // Initialize the quantum security manager
    bool initialize();

    // Shutdown the quantum security manager
    void shutdown();

    // Quantum-resistant transaction verification
    bool verifyTransaction(const std::string& transactionId) const;
    bool verifyTransactionSignature(const std::string& transactionId) const;
    bool verifyTransactionAmount(const std::string& transactionId) const;

    // Quantum-resistant asset validation
    bool validateAsset(const std::string& assetId) const;
    bool validateAssetOwnership(const std::string& assetId, const std::string& owner) const;
    bool validateAssetTransfer(const std::string& assetId, const std::string& from, const std::string& to) const;

    // Quantum-resistant key management
    bool generateQuantumKeyPair(std::string& publicKey, std::string& privateKey);
    bool storeQuantumKey(const std::string& keyId, const std::string& key, const std::string& metadata);
    bool retrieveQuantumKey(const std::string& keyId, std::string& key, std::string& metadata);
    bool rotateQuantumKey(const std::string& keyId);

    // Quantum-resistant encryption
    bool encryptData(const std::string& data, const std::string& publicKey, std::string& encryptedData);
    bool decryptData(const std::string& encryptedData, const std::string& privateKey, std::string& decryptedData);

    // Quantum-resistant signing
    bool signData(const std::string& data, const std::string& privateKey, std::string& signature);
    bool verifySignature(const std::string& data, const std::string& signature, const std::string& publicKey);

    // Quantum-resistant hash functions
    bool computeHash(const std::string& data, std::string& hash);
    bool verifyHash(const std::string& data, const std::string& hash);

    // Quantum-resistant key exchange
    bool performKeyExchange(const std::string& publicKey, std::string& sessionKey);
    bool verifyKeyExchange(const std::string& sessionKey, const std::string& privateKey);

    // Quantum-resistant random number generation
    bool generateRandomBytes(size_t length, std::vector<uint8_t>& randomBytes);
    bool generateRandomNumber(uint64_t min, uint64_t max, uint64_t& randomNumber);

    // Status and information
    bool isQuantumResistant() const;
    std::string getQuantumAlgorithm() const;
    std::string getQuantumVersion() const;
    std::vector<std::string> getAvailableAlgorithms() const;

private:
    std::unique_ptr<QuantumManager> quantumManager;
    std::unique_ptr<HybridEncryption> hybridEncryption;
    std::unique_ptr<KeyStorage> keyStorage;
    std::unique_ptr<PostQuantumAlgorithms> postQuantumAlgorithms;
    std::unique_ptr<SecurityManager> securityManager;
    mutable std::mutex mutex;
    bool initialized;

    // Internal helper functions
    bool initializeQuantumSystem();
    void cleanupQuantumSystem();
    bool validateKeyPair(const std::string& publicKey, const std::string& privateKey) const;
    bool validateMessage(const std::string& message) const;
    bool validateCiphertext(const std::string& ciphertext) const;
    bool validateSignature(const std::string& signature) const;
    bool validateHash(const std::string& hash) const;
};

} // namespace quantum
} // namespace satox 