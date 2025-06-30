/**
 * @file $(basename "$1")
 * @brief $(basename "$1" | sed 's/\./_/g' | tr '[:lower:]' '[:upper:]')
 * @copyright Copyright (c) 2025 Satoxcoin Core Developers
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <nlohmann/json.hpp>

namespace satox::core {

class QuantumManager {
public:
    // Singleton instance
    static QuantumManager& getInstance();

    // Initialization and shutdown
    bool initialize(const nlohmann::json& config = nlohmann::json::object());
    bool shutdown();
    bool isInitialized() const;

    // Quantum-resistant key generation
    struct QuantumKeyPair {
        std::vector<uint8_t> publicKey;
        std::vector<uint8_t> privateKey;
    };
    QuantumKeyPair generateKeyPair(const std::string& algorithm = "CRYSTALS-Kyber");

    // Quantum-resistant signatures
    struct QuantumSignature {
        std::vector<uint8_t> signature;
        std::vector<uint8_t> publicKey;
    };
    QuantumSignature sign(const std::vector<uint8_t>& data, const std::vector<uint8_t>& privateKey);
    bool verify(const std::vector<uint8_t>& data, const QuantumSignature& signature);

    // Quantum-resistant encryption
    struct QuantumEncryption {
        std::vector<uint8_t> ciphertext;
        std::vector<uint8_t> publicKey;
    };
    QuantumEncryption encrypt(const std::vector<uint8_t>& data, const std::vector<uint8_t>& publicKey);
    std::vector<uint8_t> decrypt(const QuantumEncryption& encryption, const std::vector<uint8_t>& privateKey);

    // Quantum random number generation
    std::vector<uint8_t> generateRandomBytes(size_t length);
    uint64_t generateRandomNumber(uint64_t min, uint64_t max);

    // Quantum-resistant hash functions
    std::vector<uint8_t> hash(const std::vector<uint8_t>& data, const std::string& algorithm = "SHA-3");

    // Supported algorithms
    enum class Algorithm {
        CRYSTALS_KYBER,    // Key encapsulation
        CRYSTALS_DILITHIUM, // Digital signatures
        FALCON,            // Digital signatures
        SPHINCS_PLUS,      // Digital signatures
        NTRU,              // Key encapsulation
        SABER,             // Key encapsulation
        SHA_3,             // Hash function
        SHAKE_128,         // Extendable output function
        SHAKE_256          // Extendable output function
    };

    // Algorithm information
    struct AlgorithmInfo {
        std::string name;
        std::string description;
        size_t keySize;
        size_t signatureSize;
        size_t ciphertextSize;
        bool isSignature;
        bool isEncryption;
        bool isHash;
    };
    AlgorithmInfo getAlgorithmInfo(Algorithm algorithm) const;
    std::vector<Algorithm> getSupportedAlgorithms() const;

    // Statistics
    struct QuantumStats {
        uint64_t totalKeyPairs;
        uint64_t totalSignatures;
        uint64_t totalVerifications;
        uint64_t totalEncryptions;
        uint64_t totalDecryptions;
        uint64_t totalRandomBytes;
        uint64_t totalHashes;
        std::unordered_map<std::string, uint64_t> algorithmUsage;
    };
    QuantumStats getStats() const;
    bool enableStats(bool enable);
    bool resetStats();

    // Error handling
    std::string getLastError() const;
    bool clearLastError();

    // Callbacks
    using ErrorCallback = std::function<void(const std::string&)>;
    using StatsCallback = std::function<void(const QuantumStats&)>;

    bool registerErrorCallback(ErrorCallback callback);
    bool registerStatsCallback(StatsCallback callback);
    bool unregisterErrorCallback();
    bool unregisterStatsCallback();

private:
    QuantumManager() = default;
    ~QuantumManager() = default;
    QuantumManager(const QuantumManager&) = delete;
    QuantumManager& operator=(const QuantumManager&) = delete;

    bool validateConfig(const nlohmann::json& config) const;
    void handleError(const std::string& error);
    void updateStats();

    // Quantum-resistant key generation methods
    QuantumKeyPair generateKyberKeyPair();
    QuantumKeyPair generateDilithiumKeyPair();
    QuantumKeyPair generateFalconKeyPair();
    QuantumKeyPair generateSphincsKeyPair();
    QuantumKeyPair generateNTRUKeyPair();
    QuantumKeyPair generateSABERKeyPair();
    QuantumKeyPair generateOpenSSLKeyPair(const std::string& algorithm);

    // Quantum-resistant signature methods
    QuantumSignature signWithDilithium(const std::vector<uint8_t>& data, const std::vector<uint8_t>& privateKey);
    QuantumSignature signWithFalcon(const std::vector<uint8_t>& data, const std::vector<uint8_t>& privateKey);
    QuantumSignature signWithSphincs(const std::vector<uint8_t>& data, const std::vector<uint8_t>& privateKey);
    QuantumSignature signWithOpenSSL(const std::vector<uint8_t>& data, const std::vector<uint8_t>& privateKey);

    // Quantum-resistant verification methods
    bool verifyWithDilithium(const std::vector<uint8_t>& data, const QuantumSignature& signature);
    bool verifyWithFalcon(const std::vector<uint8_t>& data, const QuantumSignature& signature);
    bool verifyWithSphincs(const std::vector<uint8_t>& data, const QuantumSignature& signature);
    bool verifyWithOpenSSL(const std::vector<uint8_t>& data, const QuantumSignature& signature);

    // Quantum-resistant encryption methods
    QuantumEncryption encryptWithKyber(const std::vector<uint8_t>& data, const std::vector<uint8_t>& publicKey);
    QuantumEncryption encryptWithNTRU(const std::vector<uint8_t>& data, const std::vector<uint8_t>& publicKey);
    QuantumEncryption encryptWithSABER(const std::vector<uint8_t>& data, const std::vector<uint8_t>& publicKey);
    QuantumEncryption encryptWithOpenSSL(const std::vector<uint8_t>& data, const std::vector<uint8_t>& publicKey);

    // Quantum-resistant decryption methods
    std::vector<uint8_t> decryptWithKyber(const QuantumEncryption& encryption, const std::vector<uint8_t>& privateKey);
    std::vector<uint8_t> decryptWithNTRU(const QuantumEncryption& encryption, const std::vector<uint8_t>& privateKey);
    std::vector<uint8_t> decryptWithSABER(const QuantumEncryption& encryption, const std::vector<uint8_t>& privateKey);
    std::vector<uint8_t> decryptWithOpenSSL(const QuantumEncryption& encryption, const std::vector<uint8_t>& privateKey);

    // Quantum-resistant hash methods
    std::vector<uint8_t> hashWithSHA3(const std::vector<uint8_t>& data);
    std::vector<uint8_t> hashWithSHAKE128(const std::vector<uint8_t>& data);
    std::vector<uint8_t> hashWithSHAKE256(const std::vector<uint8_t>& data);

    bool initialized_;
    std::string lastError_;
    nlohmann::json config_;
    QuantumStats stats_;
    bool statsEnabled_;

    ErrorCallback errorCallback_;
    StatsCallback statsCallback_;

    mutable std::mutex mutex_;
};

} // namespace satox::core 