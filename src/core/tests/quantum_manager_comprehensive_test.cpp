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
#include <thread>
#include <chrono>
#include <random>
#include <fstream>
#include <filesystem>

using namespace satox::core;

class QuantumManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &QuantumManager::getInstance();
        manager->shutdown(); // Ensure clean state
    }

    void TearDown() override {
        manager->shutdown();
    }

    QuantumManager* manager;
};

// Initialization Tests
TEST_F(QuantumManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Should fail on second init
    EXPECT_TRUE(manager->isInitialized());
}

// Key Generation Tests
TEST_F(QuantumManagerTest, KeyGeneration) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    EXPECT_FALSE(keyPair.publicKey.empty());
    EXPECT_FALSE(keyPair.privateKey.empty());
    
    // Generate key pair with specific algorithm
    keyPair = manager->generateKeyPair("CRYSTALS-Kyber");
    EXPECT_FALSE(keyPair.publicKey.empty());
    EXPECT_FALSE(keyPair.privateKey.empty());
}

// Signature Tests
TEST_F(QuantumManagerTest, Signatures) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    
    // Sign data
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto signature = manager->sign(data, keyPair.privateKey);
    EXPECT_FALSE(signature.signature.empty());
    EXPECT_FALSE(signature.publicKey.empty());
    
    // Verify signature
    EXPECT_TRUE(manager->verify(data, signature));
    
    // Verify with modified data
    data[0] = 0;
    EXPECT_FALSE(manager->verify(data, signature));
}

// Encryption Tests
TEST_F(QuantumManagerTest, Encryption) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    
    // Encrypt data
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto encryption = manager->encrypt(data, keyPair.publicKey);
    EXPECT_FALSE(encryption.ciphertext.empty());
    EXPECT_FALSE(encryption.publicKey.empty());
    
    // Decrypt data
    auto decrypted = manager->decrypt(encryption, keyPair.privateKey);
    EXPECT_EQ(decrypted, data);
}

// Random Number Generation Tests
TEST_F(QuantumManagerTest, RandomNumberGeneration) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Generate random bytes
    auto randomBytes = manager->generateRandomBytes(32);
    EXPECT_EQ(randomBytes.size(), 32);
    
    // Generate random number
    auto randomNumber = manager->generateRandomNumber(0, 100);
    EXPECT_GE(randomNumber, 0);
    EXPECT_LE(randomNumber, 100);
}

// Hash Function Tests
TEST_F(QuantumManagerTest, HashFunctions) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Hash data
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto hash = manager->hash(data);
    EXPECT_FALSE(hash.empty());
    
    // Hash with specific algorithm
    hash = manager->hash(data, "SHA-3");
    EXPECT_FALSE(hash.empty());
}

// Algorithm Information Tests
TEST_F(QuantumManagerTest, AlgorithmInformation) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Get algorithm info
    auto info = manager->getAlgorithmInfo(QuantumManager::Algorithm::CRYSTALS_KYBER);
    EXPECT_FALSE(info.name.empty());
    EXPECT_FALSE(info.description.empty());
    EXPECT_GT(info.keySize, 0);
    
    // Get supported algorithms
    auto algorithms = manager->getSupportedAlgorithms();
    EXPECT_FALSE(algorithms.empty());
}

// Statistics Tests
TEST_F(QuantumManagerTest, Statistics) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Enable statistics
    EXPECT_TRUE(manager->enableStats(true));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    
    // Sign data
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto signature = manager->sign(data, keyPair.privateKey);
    
    // Verify signature
    EXPECT_TRUE(manager->verify(data, signature));
    
    // Encrypt data
    auto encryption = manager->encrypt(data, keyPair.publicKey);
    
    // Decrypt data
    auto decrypted = manager->decrypt(encryption, keyPair.privateKey);
    
    // Generate random bytes
    auto randomBytes = manager->generateRandomBytes(32);
    
    // Hash data
    auto hash = manager->hash(data);
    
    // Get stats
    auto stats = manager->getStats();
    EXPECT_GT(stats.totalKeyPairs, 0);
    EXPECT_GT(stats.totalSignatures, 0);
    EXPECT_GT(stats.totalVerifications, 0);
    EXPECT_GT(stats.totalEncryptions, 0);
    EXPECT_GT(stats.totalDecryptions, 0);
    EXPECT_GT(stats.totalRandomBytes, 0);
    EXPECT_GT(stats.totalHashes, 0);
    EXPECT_FALSE(stats.algorithmUsage.empty());
    
    // Reset stats
    EXPECT_TRUE(manager->resetStats());
    stats = manager->getStats();
    EXPECT_EQ(stats.totalKeyPairs, 0);
    EXPECT_EQ(stats.totalSignatures, 0);
    EXPECT_EQ(stats.totalVerifications, 0);
    EXPECT_EQ(stats.totalEncryptions, 0);
    EXPECT_EQ(stats.totalDecryptions, 0);
    EXPECT_EQ(stats.totalRandomBytes, 0);
    EXPECT_EQ(stats.totalHashes, 0);
    EXPECT_TRUE(stats.algorithmUsage.empty());
}

// Callback Tests
TEST_F(QuantumManagerTest, Callbacks) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Register callbacks
    bool errorCallbackCalled = false;
    bool statsCallbackCalled = false;
    
    EXPECT_TRUE(manager->registerErrorCallback([&](const std::string& error) {
        errorCallbackCalled = true;
    }));
    
    EXPECT_TRUE(manager->registerStatsCallback([&](const QuantumManager::QuantumStats& stats) {
        statsCallbackCalled = true;
    }));
    
    // Enable stats
    EXPECT_TRUE(manager->enableStats(true));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    EXPECT_TRUE(statsCallbackCalled);
    
    // Unregister callbacks
    EXPECT_TRUE(manager->unregisterErrorCallback());
    EXPECT_TRUE(manager->unregisterStatsCallback());
    
    errorCallbackCalled = false;
    statsCallbackCalled = false;
    
    // Generate key pair
    keyPair = manager->generateKeyPair();
    EXPECT_FALSE(errorCallbackCalled);
    EXPECT_FALSE(statsCallbackCalled);
}

// Error Handling Tests
TEST_F(QuantumManagerTest, ErrorHandling) {
    // Test invalid initialization
    EXPECT_FALSE(manager->generateKeyPair());
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test invalid key pair
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    std::vector<uint8_t> invalidKey = {1, 2, 3};
    EXPECT_FALSE(manager->sign({1, 2, 3}, invalidKey));
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test clear error
    EXPECT_TRUE(manager->clearLastError());
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(QuantumManagerTest, Concurrency) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    const int numThreads = 10;
    const int numOperationsPerThread = 100;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, numOperationsPerThread]() {
            for (int j = 0; j < numOperationsPerThread; ++j) {
                // Generate key pair
                auto keyPair = manager->generateKeyPair();
                
                // Sign data
                std::vector<uint8_t> data = {1, 2, 3, 4, 5};
                auto signature = manager->sign(data, keyPair.privateKey);
                
                // Verify signature
                EXPECT_TRUE(manager->verify(data, signature));
                
                // Encrypt data
                auto encryption = manager->encrypt(data, keyPair.publicKey);
                
                // Decrypt data
                auto decrypted = manager->decrypt(encryption, keyPair.privateKey);
                
                // Generate random bytes
                auto randomBytes = manager->generateRandomBytes(32);
                
                // Hash data
                auto hash = manager->hash(data);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

// Edge Cases Tests
TEST_F(QuantumManagerTest, EdgeCases) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Test empty data
    std::vector<uint8_t> emptyData;
    auto keyPair = manager->generateKeyPair();
    auto signature = manager->sign(emptyData, keyPair.privateKey);
    EXPECT_TRUE(manager->verify(emptyData, signature));
    
    // Test large data
    std::vector<uint8_t> largeData(1024 * 1024, 1); // 1MB
    signature = manager->sign(largeData, keyPair.privateKey);
    EXPECT_TRUE(manager->verify(largeData, signature));
    
    // Test invalid range for random number
    EXPECT_EQ(manager->generateRandomNumber(100, 0), 0);
}

// Cleanup Tests
TEST_F(QuantumManagerTest, Cleanup) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    // Generate key pair
    auto keyPair = manager->generateKeyPair();
    
    // Sign data
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    auto signature = manager->sign(data, keyPair.privateKey);
    
    // Shutdown
    EXPECT_TRUE(manager->shutdown());
    EXPECT_FALSE(manager->isInitialized());
}

// Stress Tests
TEST_F(QuantumManagerTest, StressTest) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    
    const int numOperations = 1000;
    for (int i = 0; i < numOperations; ++i) {
        // Generate key pair
        auto keyPair = manager->generateKeyPair();
        
        // Sign data
        std::vector<uint8_t> data = {1, 2, 3, 4, 5};
        auto signature = manager->sign(data, keyPair.privateKey);
        
        // Verify signature
        EXPECT_TRUE(manager->verify(data, signature));
        
        // Encrypt data
        auto encryption = manager->encrypt(data, keyPair.publicKey);
        
        // Decrypt data
        auto decrypted = manager->decrypt(encryption, keyPair.privateKey);
        
        // Generate random bytes
        auto randomBytes = manager->generateRandomBytes(32);
        
        // Hash data
        auto hash = manager->hash(data);
    }
} 