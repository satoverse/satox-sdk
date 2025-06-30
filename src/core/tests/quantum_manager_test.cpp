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

#include <gtest/gtest.h>
#include "satox/core/quantum_manager.hpp"
#include <vector>
#include <string>

class QuantumManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize quantum manager before each test
        nlohmann::json config;
        config["enable_stats"] = true;
        config["default_algorithm"] = "CRYSTALS-Kyber";
        
        ASSERT_TRUE(quantumManager.initialize(config));
    }

    void TearDown() override {
        // Shutdown quantum manager after each test
        quantumManager.shutdown();
    }

    satox::core::QuantumManager& quantumManager = satox::core::QuantumManager::getInstance();
};

TEST_F(QuantumManagerTest, Initialization) {
    // Test initialization
    EXPECT_TRUE(quantumManager.isInitialized());
    
    // Test shutdown and re-initialization
    EXPECT_TRUE(quantumManager.shutdown());
    EXPECT_FALSE(quantumManager.isInitialized());
    
    nlohmann::json config;
    EXPECT_TRUE(quantumManager.initialize(config));
    EXPECT_TRUE(quantumManager.isInitialized());
}

TEST_F(QuantumManagerTest, KeyGeneration) {
    // Test Kyber key generation
    auto kyberKeys = quantumManager.generateKeyPair("CRYSTALS-Kyber");
    EXPECT_FALSE(kyberKeys.publicKey.empty());
    EXPECT_FALSE(kyberKeys.privateKey.empty());
    EXPECT_EQ(kyberKeys.publicKey.size(), 1186); // 1184 + 2 byte algorithm ID
    EXPECT_EQ(kyberKeys.privateKey.size(), 2402); // 2400 + 2 byte algorithm ID
    
    // Test Dilithium key generation
    auto dilithiumKeys = quantumManager.generateKeyPair("CRYSTALS-Dilithium");
    EXPECT_FALSE(dilithiumKeys.publicKey.empty());
    EXPECT_FALSE(dilithiumKeys.privateKey.empty());
    EXPECT_EQ(dilithiumKeys.publicKey.size(), 1954); // 1952 + 2 byte algorithm ID
    EXPECT_EQ(dilithiumKeys.privateKey.size(), 4002); // 4000 + 2 byte algorithm ID
    
    // Test Falcon key generation
    auto falconKeys = quantumManager.generateKeyPair("Falcon");
    EXPECT_FALSE(falconKeys.publicKey.empty());
    EXPECT_FALSE(falconKeys.privateKey.empty());
    EXPECT_EQ(falconKeys.publicKey.size(), 1795); // 1793 + 2 byte algorithm ID
    EXPECT_EQ(falconKeys.privateKey.size(), 2307); // 2305 + 2 byte algorithm ID
    
    // Test SPHINCS+ key generation
    auto sphincsKeys = quantumManager.generateKeyPair("SPHINCS+");
    EXPECT_FALSE(sphincsKeys.publicKey.empty());
    EXPECT_FALSE(sphincsKeys.privateKey.empty());
    EXPECT_EQ(sphincsKeys.publicKey.size(), 66); // 64 + 2 byte algorithm ID
    EXPECT_EQ(sphincsKeys.privateKey.size(), 98); // 96 + 2 byte algorithm ID
    
    // Test NTRU key generation
    auto ntruKeys = quantumManager.generateKeyPair("NTRU");
    EXPECT_FALSE(ntruKeys.publicKey.empty());
    EXPECT_FALSE(ntruKeys.privateKey.empty());
    EXPECT_EQ(ntruKeys.publicKey.size(), 1458); // 1456 + 2 byte algorithm ID
    EXPECT_EQ(ntruKeys.privateKey.size(), 2050); // 2048 + 2 byte algorithm ID
    
    // Test SABER key generation
    auto saberKeys = quantumManager.generateKeyPair("SABER");
    EXPECT_FALSE(saberKeys.publicKey.empty());
    EXPECT_FALSE(saberKeys.privateKey.empty());
    EXPECT_EQ(saberKeys.publicKey.size(), 994); // 992 + 2 byte algorithm ID
    EXPECT_EQ(saberKeys.privateKey.size(), 2306); // 2304 + 2 byte algorithm ID
}

TEST_F(QuantumManagerTest, SigningAndVerification) {
    // Test Dilithium signing and verification
    auto dilithiumKeys = quantumManager.generateKeyPair("CRYSTALS-Dilithium");
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    
    auto signature = quantumManager.sign(testData, dilithiumKeys.privateKey);
    EXPECT_FALSE(signature.signature.empty());
    EXPECT_FALSE(signature.publicKey.empty());
    EXPECT_EQ(signature.signature.size(), 3366); // Dilithium-5 signature size
    
    bool verified = quantumManager.verify(testData, signature);
    EXPECT_TRUE(verified);
    
    // Test Falcon signing and verification
    auto falconKeys = quantumManager.generateKeyPair("Falcon");
    auto falconSignature = quantumManager.sign(testData, falconKeys.privateKey);
    EXPECT_FALSE(falconSignature.signature.empty());
    EXPECT_EQ(falconSignature.signature.size(), 1281); // Falcon-1024 signature size
    
    bool falconVerified = quantumManager.verify(testData, falconSignature);
    EXPECT_TRUE(falconVerified);
    
    // Test SPHINCS+ signing and verification
    auto sphincsKeys = quantumManager.generateKeyPair("SPHINCS+");
    auto sphincsSignature = quantumManager.sign(testData, sphincsKeys.privateKey);
    EXPECT_FALSE(sphincsSignature.signature.empty());
    EXPECT_EQ(sphincsSignature.signature.size(), 49856); // SPHINCS+ signature size
    
    bool sphincsVerified = quantumManager.verify(testData, sphincsSignature);
    EXPECT_TRUE(sphincsVerified);
}

TEST_F(QuantumManagerTest, EncryptionAndDecryption) {
    // Test Kyber encryption and decryption
    auto kyberKeys = quantumManager.generateKeyPair("CRYSTALS-Kyber");
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto encryption = quantumManager.encrypt(testData, kyberKeys.publicKey);
    EXPECT_FALSE(encryption.ciphertext.empty());
    EXPECT_FALSE(encryption.publicKey.empty());
    EXPECT_EQ(encryption.ciphertext.size(), 1088); // Kyber-1024 ciphertext size
    
    auto decrypted = quantumManager.decrypt(encryption, kyberKeys.privateKey);
    EXPECT_FALSE(decrypted.empty());
    EXPECT_EQ(decrypted.size(), 32); // Expected shared secret size
    
    // Test NTRU encryption and decryption
    auto ntruKeys = quantumManager.generateKeyPair("NTRU");
    auto ntruEncryption = quantumManager.encrypt(testData, ntruKeys.publicKey);
    EXPECT_FALSE(ntruEncryption.ciphertext.empty());
    EXPECT_EQ(ntruEncryption.ciphertext.size(), 1456); // NTRU ciphertext size
    
    auto ntruDecrypted = quantumManager.decrypt(ntruEncryption, ntruKeys.privateKey);
    EXPECT_FALSE(ntruDecrypted.empty());
    
    // Test SABER encryption and decryption
    auto saberKeys = quantumManager.generateKeyPair("SABER");
    auto saberEncryption = quantumManager.encrypt(testData, saberKeys.publicKey);
    EXPECT_FALSE(saberEncryption.ciphertext.empty());
    EXPECT_EQ(saberEncryption.ciphertext.size(), 1088); // SABER ciphertext size
    
    auto saberDecrypted = quantumManager.decrypt(saberEncryption, saberKeys.privateKey);
    EXPECT_FALSE(saberDecrypted.empty());
}

TEST_F(QuantumManagerTest, HashFunctions) {
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Test SHA-3 hash
    auto sha3Hash = quantumManager.hash(testData, "SHA-3");
    EXPECT_FALSE(sha3Hash.empty());
    EXPECT_EQ(sha3Hash.size(), 32); // SHA-3-256 produces 32 bytes
    
    // Test SHAKE-128 hash
    auto shake128Hash = quantumManager.hash(testData, "SHAKE-128");
    EXPECT_FALSE(shake128Hash.empty());
    EXPECT_EQ(shake128Hash.size(), 32); // We configured it to produce 32 bytes
    
    // Test SHAKE-256 hash
    auto shake256Hash = quantumManager.hash(testData, "SHAKE-256");
    EXPECT_EQ(shake256Hash.size(), 32); // We configured it to produce 32 bytes
    
    // Test default hash (should be SHA-3)
    auto defaultHash = quantumManager.hash(testData);
    EXPECT_FALSE(defaultHash.empty());
    EXPECT_EQ(defaultHash.size(), 32);
}

TEST_F(QuantumManagerTest, RandomNumberGeneration) {
    // Test random bytes generation
    auto randomBytes = quantumManager.generateRandomBytes(64);
    EXPECT_EQ(randomBytes.size(), 64);
    
    // Test random number generation
    uint64_t randomNum = quantumManager.generateRandomNumber(1, 100);
    EXPECT_GE(randomNum, 1);
    EXPECT_LE(randomNum, 100);
    
    // Test range validation
    uint64_t invalidNum = quantumManager.generateRandomNumber(100, 1);
    EXPECT_EQ(invalidNum, 0); // Should return 0 for invalid range
}

TEST_F(QuantumManagerTest, AlgorithmInfo) {
    // Test algorithm information retrieval
    auto kyberInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::CRYSTALS_KYBER);
    EXPECT_EQ(kyberInfo.name, "CRYSTALS-Kyber");
    EXPECT_TRUE(kyberInfo.isEncryption);
    EXPECT_FALSE(kyberInfo.isSignature);
    
    auto dilithiumInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::CRYSTALS_DILITHIUM);
    EXPECT_EQ(dilithiumInfo.name, "CRYSTALS-Dilithium");
    EXPECT_TRUE(dilithiumInfo.isSignature);
    EXPECT_FALSE(dilithiumInfo.isEncryption);
    
    auto falconInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::FALCON);
    EXPECT_EQ(falconInfo.name, "FALCON");
    EXPECT_TRUE(falconInfo.isSignature);
    
    auto sphincsInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::SPHINCS_PLUS);
    EXPECT_EQ(sphincsInfo.name, "SPHINCS+");
    EXPECT_TRUE(sphincsInfo.isSignature);
    
    auto ntruInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::NTRU);
    EXPECT_EQ(ntruInfo.name, "NTRU");
    EXPECT_TRUE(ntruInfo.isEncryption);
    
    auto saberInfo = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::SABER);
    EXPECT_EQ(saberInfo.name, "SABER");
    EXPECT_TRUE(saberInfo.isEncryption);
    
    auto sha3Info = quantumManager.getAlgorithmInfo(satox::core::QuantumManager::Algorithm::SHA_3);
    EXPECT_EQ(sha3Info.name, "SHA-3");
    EXPECT_TRUE(sha3Info.isHash);
}

TEST_F(QuantumManagerTest, Statistics) {
    // Enable statistics
    EXPECT_TRUE(quantumManager.enableStats(true));
    
    // Perform some operations to generate statistics
    auto keys = quantumManager.generateKeyPair("CRYSTALS-Kyber");
    std::vector<uint8_t> testData = {1, 2, 3, 4, 5};
    auto signature = quantumManager.sign(testData, keys.privateKey);
    quantumManager.verify(testData, signature);
    auto encryption = quantumManager.encrypt(testData, keys.publicKey);
    quantumManager.decrypt(encryption, keys.privateKey);
    quantumManager.hash(testData);
    quantumManager.generateRandomBytes(32);
    
    // Get statistics
    auto stats = quantumManager.getStats();
    EXPECT_GT(stats.totalKeyPairs, 0);
    EXPECT_GT(stats.totalSignatures, 0);
    EXPECT_GT(stats.totalVerifications, 0);
    EXPECT_GT(stats.totalEncryptions, 0);
    EXPECT_GT(stats.totalDecryptions, 0);
    EXPECT_GT(stats.totalHashes, 0);
    EXPECT_GT(stats.totalRandomBytes, 0);
    EXPECT_GT(stats.algorithmUsage.size(), 0);
    
    // Test reset statistics
    EXPECT_TRUE(quantumManager.resetStats());
    auto resetStats = quantumManager.getStats();
    EXPECT_EQ(resetStats.totalKeyPairs, 0);
    EXPECT_EQ(resetStats.totalSignatures, 0);
    EXPECT_EQ(resetStats.totalVerifications, 0);
    EXPECT_EQ(resetStats.totalEncryptions, 0);
    EXPECT_EQ(resetStats.totalDecryptions, 0);
    EXPECT_EQ(resetStats.totalHashes, 0);
    EXPECT_EQ(resetStats.totalRandomBytes, 0);
}

TEST_F(QuantumManagerTest, ErrorHandling) {
    // Test error handling for uninitialized manager
    quantumManager.shutdown();
    
    auto keys = quantumManager.generateKeyPair("CRYSTALS-Kyber");
    EXPECT_TRUE(keys.publicKey.empty());
    EXPECT_TRUE(keys.privateKey.empty());
    
    std::string error = quantumManager.getLastError();
    EXPECT_FALSE(error.empty());
    EXPECT_EQ(error, "QuantumManager not initialized");
    
    // Test error clearing
    EXPECT_TRUE(quantumManager.clearLastError());
    error = quantumManager.getLastError();
    EXPECT_TRUE(error.empty());
    
    // Re-initialize for cleanup
    quantumManager.initialize();
}

TEST_F(QuantumManagerTest, Callbacks) {
    bool errorCallbackCalled = false;
    bool statsCallbackCalled = false;
    
    // Register error callback
    auto errorCallback = [&errorCallbackCalled](const std::string& error) {
        errorCallbackCalled = true;
    };
    EXPECT_TRUE(quantumManager.registerErrorCallback(errorCallback));
    
    // Register stats callback
    auto statsCallback = [&statsCallbackCalled](const satox::core::QuantumManager::QuantumStats& stats) {
        statsCallbackCalled = true;
    };
    EXPECT_TRUE(quantumManager.registerStatsCallback(statsCallback));
    
    // Trigger an error to test error callback
    quantumManager.shutdown();
    quantumManager.generateKeyPair("CRYSTALS-Kyber");
    EXPECT_TRUE(errorCallbackCalled);
    
    // Re-initialize and trigger stats update
    quantumManager.initialize();
    quantumManager.enableStats(true);
    quantumManager.generateKeyPair("CRYSTALS-Kyber");
    // Note: Stats callback is called internally, we can't easily test it here
    
    // Test unregistering callbacks
    EXPECT_TRUE(quantumManager.unregisterErrorCallback());
    EXPECT_TRUE(quantumManager.unregisterStatsCallback());
}

TEST_F(QuantumManagerTest, SupportedAlgorithms) {
    auto algorithms = quantumManager.getSupportedAlgorithms();
    EXPECT_FALSE(algorithms.empty());
    
    // Check that all expected algorithms are present
    bool hasKyber = false, hasDilithium = false, hasFalcon = false;
    bool hasSphincs = false, hasNTRU = false, hasSABER = false;
    bool hasSHA3 = false, hasSHAKE128 = false, hasSHAKE256 = false;
    
    for (const auto& algo : algorithms) {
        switch (algo) {
            case satox::core::QuantumManager::Algorithm::CRYSTALS_KYBER:
                hasKyber = true;
                break;
            case satox::core::QuantumManager::Algorithm::CRYSTALS_DILITHIUM:
                hasDilithium = true;
                break;
            case satox::core::QuantumManager::Algorithm::FALCON:
                hasFalcon = true;
                break;
            case satox::core::QuantumManager::Algorithm::SPHINCS_PLUS:
                hasSphincs = true;
                break;
            case satox::core::QuantumManager::Algorithm::NTRU:
                hasNTRU = true;
                break;
            case satox::core::QuantumManager::Algorithm::SABER:
                hasSABER = true;
                break;
            case satox::core::QuantumManager::Algorithm::SHA_3:
                hasSHA3 = true;
                break;
            case satox::core::QuantumManager::Algorithm::SHAKE_128:
                hasSHAKE128 = true;
                break;
            case satox::core::QuantumManager::Algorithm::SHAKE_256:
                hasSHAKE256 = true;
                break;
        }
    }
    
    EXPECT_TRUE(hasKyber);
    EXPECT_TRUE(hasDilithium);
    EXPECT_TRUE(hasFalcon);
    EXPECT_TRUE(hasSphincs);
    EXPECT_TRUE(hasNTRU);
    EXPECT_TRUE(hasSABER);
    EXPECT_TRUE(hasSHA3);
    EXPECT_TRUE(hasSHAKE128);
    EXPECT_TRUE(hasSHAKE256);
} 