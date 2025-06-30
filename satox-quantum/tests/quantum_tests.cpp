#include <gtest/gtest.h>
#include <satox/quantum/quantum_manager.hpp>
#include <satox/quantum/hybrid_encryption.hpp>
#include <satox/quantum/key_storage.hpp>
#include <satox/quantum/post_quantum_algorithms.hpp>
#include <satox/quantum/quantum_security_manager.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <sodium.h>

using namespace satox::quantum;

class QuantumTests : public ::testing::Test {
protected:
    void SetUp() override {
        quantumManager = std::make_unique<QuantumManager>();
        hybridEncryption = std::make_unique<HybridEncryption>();
        keyStorage = std::make_unique<KeyStorage>();
        postQuantumAlgorithms = std::make_unique<PostQuantumAlgorithms>();
        quantumSecurityManager = std::make_unique<QuantumSecurityManager>();

        ASSERT_TRUE(quantumManager->initialize());
        ASSERT_TRUE(hybridEncryption->initialize());
        ASSERT_TRUE(keyStorage->initialize());
        ASSERT_TRUE(postQuantumAlgorithms->initialize());
        ASSERT_TRUE(quantumSecurityManager->initialize());
    }

    void TearDown() override {
        quantumManager->shutdown();
        hybridEncryption->shutdown();
        keyStorage->shutdown();
        postQuantumAlgorithms->shutdown();
        quantumSecurityManager->shutdown();
    }

    std::unique_ptr<QuantumManager> quantumManager;
    std::unique_ptr<HybridEncryption> hybridEncryption;
    std::unique_ptr<KeyStorage> keyStorage;
    std::unique_ptr<PostQuantumAlgorithms> postQuantumAlgorithms;
    std::unique_ptr<QuantumSecurityManager> quantumSecurityManager;
};

// Quantum Manager Tests
TEST_F(QuantumTests, QuantumManagerInitialization) {
    EXPECT_TRUE(quantumManager->isInitialized());
    EXPECT_TRUE(quantumManager->isQuantumResistant());
    EXPECT_FALSE(quantumManager->getQuantumAlgorithm().empty());
    EXPECT_FALSE(quantumManager->getQuantumVersion().empty());
}

TEST_F(QuantumTests, QuantumKeyGeneration) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
}

TEST_F(QuantumTests, QuantumEncryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string encryptedData, decryptedData;
    EXPECT_TRUE(quantumManager->encryptWithQuantumKey(message, publicKey, encryptedData));
    EXPECT_TRUE(quantumManager->decryptWithQuantumKey(encryptedData, privateKey, decryptedData));
    EXPECT_EQ(message, decryptedData);
}

// Hybrid Encryption Tests
TEST_F(QuantumTests, HybridEncryptionInitialization) {
    EXPECT_TRUE(hybridEncryption->isInitialized());
}

TEST_F(QuantumTests, HybridEncryption) {
    std::string quantumPublicKey, quantumPrivateKey;
    std::string classicalPublicKey, classicalPrivateKey;
    ASSERT_TRUE(quantumManager->generateQuantumKeyPair(quantumPublicKey, quantumPrivateKey));
    
    // Generate classical key pair using libsodium
    unsigned char publicKey[crypto_box_PUBLICKEYBYTES];
    unsigned char privateKey[crypto_box_SECRETKEYBYTES];
    crypto_box_keypair(publicKey, privateKey);
    
    classicalPublicKey = base64Encode(publicKey, crypto_box_PUBLICKEYBYTES);
    classicalPrivateKey = base64Encode(privateKey, crypto_box_SECRETKEYBYTES);

    std::string message = "Test message";
    std::string encryptedData, decryptedData;
    EXPECT_TRUE(hybridEncryption->encrypt(message, quantumPublicKey, classicalPublicKey, encryptedData));
    EXPECT_TRUE(hybridEncryption->decrypt(encryptedData, quantumPrivateKey, classicalPrivateKey, decryptedData));
    EXPECT_EQ(message, decryptedData);
}

// Key Storage Tests
TEST_F(QuantumTests, KeyStorageInitialization) {
    EXPECT_TRUE(keyStorage->isInitialized());
}

TEST_F(QuantumTests, KeyStorageOperations) {
    std::string key = "test_key";
    std::string metadata = "test_metadata";
    EXPECT_TRUE(keyStorage->storeKey(key, "key1", metadata));

    std::string retrievedKey, retrievedMetadata;
    EXPECT_TRUE(keyStorage->retrieveKey("key1", retrievedKey, retrievedMetadata));
    EXPECT_EQ(key, retrievedKey);
    EXPECT_EQ(metadata, retrievedMetadata);

    auto expiration = std::chrono::system_clock::now() + std::chrono::hours(24);
    EXPECT_TRUE(keyStorage->setKeyExpiration("key1", expiration));
    EXPECT_FALSE(keyStorage->isKeyExpired("key1"));

    std::vector<std::string> allowedUsers = {"user1", "user2"};
    EXPECT_TRUE(keyStorage->setKeyAccess("key1", allowedUsers));
    EXPECT_TRUE(keyStorage->checkKeyAccess("key1", "user1"));
    EXPECT_FALSE(keyStorage->checkKeyAccess("key1", "user3"));
}

// Post-Quantum Algorithms Tests
TEST_F(QuantumTests, PostQuantumAlgorithmsInitialization) {
    EXPECT_TRUE(postQuantumAlgorithms->isInitialized());
    auto algorithms = postQuantumAlgorithms->getAvailableAlgorithms();
    EXPECT_FALSE(algorithms.empty());
}

TEST_F(QuantumTests, CRYSTALSKyber) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateKyberKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->kyberEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->kyberDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(QuantumTests, NTRU) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateNTRUKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->ntruEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->ntruDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(QuantumTests, SABER) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateSaberKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->saberEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->saberDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(QuantumTests, ClassicMcEliece) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateMcElieceKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->mcelieceEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->mcelieceDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(QuantumTests, BIKE) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);

    // Test with different message lengths
    std::string longMessage(1000, 'A');
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(longMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(longMessage, decryptedMessage);

    // Test with binary data
    std::string binaryMessage;
    binaryMessage.reserve(256);
    for (int i = 0; i < 256; ++i) {
        binaryMessage.push_back(static_cast<char>(i));
    }
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(binaryMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(binaryMessage, decryptedMessage);
}

TEST_F(QuantumTests, HQC) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);

    // Test with different message lengths
    std::string longMessage(1000, 'A');
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(longMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(longMessage, decryptedMessage);

    // Test with binary data
    std::string binaryMessage;
    binaryMessage.reserve(256);
    for (int i = 0; i < 256; ++i) {
        binaryMessage.push_back(static_cast<char>(i));
    }
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(binaryMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(binaryMessage, decryptedMessage);
}

TEST_F(QuantumTests, SIKE) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(message, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);

    // Test with different message lengths
    std::string longMessage(1000, 'A');
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(longMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(longMessage, decryptedMessage);

    // Test with binary data
    std::string binaryMessage;
    binaryMessage.reserve(256);
    for (int i = 0; i < 256; ++i) {
        binaryMessage.push_back(static_cast<char>(i));
    }
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(binaryMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(binaryMessage, decryptedMessage);
}

// Quantum Security Manager Tests
TEST_F(QuantumTests, QuantumSecurityManagerInitialization) {
    EXPECT_TRUE(quantumSecurityManager->isInitialized());
    EXPECT_TRUE(quantumSecurityManager->isQuantumResistant());
}

TEST_F(QuantumTests, QuantumSecurityManagerOperations) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(quantumSecurityManager->generateQuantumKeyPair(publicKey, privateKey));

    std::string message = "Test message";
    std::string encryptedData, decryptedData;
    EXPECT_TRUE(quantumSecurityManager->encryptData(message, publicKey, encryptedData));
    EXPECT_TRUE(quantumSecurityManager->decryptData(encryptedData, privateKey, decryptedData));
    EXPECT_EQ(message, decryptedData);

    std::string signature;
    EXPECT_TRUE(quantumSecurityManager->signData(message, privateKey, signature));
    EXPECT_TRUE(quantumSecurityManager->verifySignature(message, signature, publicKey));

    std::string hash;
    EXPECT_TRUE(quantumSecurityManager->computeHash(message, hash));
    EXPECT_TRUE(quantumSecurityManager->verifyHash(message, hash));

    std::string sessionKey;
    EXPECT_TRUE(quantumSecurityManager->performKeyExchange(publicKey, sessionKey));
    EXPECT_TRUE(quantumSecurityManager->verifyKeyExchange(sessionKey, privateKey));
}

// Performance Tests
TEST_F(QuantumTests, Performance) {
    const int numOperations = 1000;
    std::string publicKey, privateKey;
    ASSERT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));

    // Measure encryption/decryption performance
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numOperations; ++i) {
        std::string message = "Test message " + std::to_string(i);
        std::string encryptedData, decryptedData;
        EXPECT_TRUE(quantumManager->encryptWithQuantumKey(message, publicKey, encryptedData));
        EXPECT_TRUE(quantumManager->decryptWithQuantumKey(encryptedData, privateKey, decryptedData));
        EXPECT_EQ(message, decryptedData);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Average time per encryption/decryption: " 
              << duration.count() / (2.0 * numOperations) << " ms" << std::endl;

    // Measure key generation performance
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numOperations; ++i) {
        EXPECT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Average time per key generation: " 
              << duration.count() / static_cast<double>(numOperations) << " ms" << std::endl;
}

// Thread Safety Tests
TEST_F(QuantumTests, ThreadSafety) {
    const int numThreads = 4;
    const int operationsPerThread = 1000;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([this, i, operationsPerThread]() {
            std::string publicKey, privateKey;
            ASSERT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));

            for (int j = 0; j < operationsPerThread; ++j) {
                std::string message = "Thread " + std::to_string(i) + " Message " + std::to_string(j);
                std::string encryptedData, decryptedData;
                EXPECT_TRUE(quantumManager->encryptWithQuantumKey(message, publicKey, encryptedData));
                EXPECT_TRUE(quantumManager->decryptWithQuantumKey(encryptedData, privateKey, decryptedData));
                EXPECT_EQ(message, decryptedData);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// Error Handling Tests
TEST_F(QuantumTests, ErrorHandling) {
    // Test with invalid keys
    std::string invalidKey = "invalid_key";
    std::string message = "Test message";
    std::string encryptedData, decryptedData;
    EXPECT_FALSE(quantumManager->encryptWithQuantumKey(message, invalidKey, encryptedData));
    EXPECT_FALSE(quantumManager->decryptWithQuantumKey(encryptedData, invalidKey, decryptedData));

    // Test with empty message
    std::string publicKey, privateKey;
    ASSERT_TRUE(quantumManager->generateQuantumKeyPair(publicKey, privateKey));
    EXPECT_FALSE(quantumManager->encryptWithQuantumKey("", publicKey, encryptedData));

    // Test with invalid ciphertext
    EXPECT_FALSE(quantumManager->decryptWithQuantumKey("invalid_ciphertext", privateKey, decryptedData));
}

// Additional performance tests for new algorithms
TEST_F(QuantumTests, AlgorithmPerformance) {
    const int numOperations = 100;
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    // Test BIKE performance
    {
        std::string publicKey, privateKey;
        ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numOperations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "BIKE average time per encryption/decryption: " 
                  << duration.count() / (2.0 * numOperations) << " ms" << std::endl;
    }

    // Test HQC performance
    {
        std::string publicKey, privateKey;
        ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numOperations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "HQC average time per encryption/decryption: " 
                  << duration.count() / (2.0 * numOperations) << " ms" << std::endl;
    }

    // Test SIKE performance
    {
        std::string publicKey, privateKey;
        ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numOperations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "SIKE average time per encryption/decryption: " 
                  << duration.count() / (2.0 * numOperations) << " ms" << std::endl;
    }
}

// Additional error handling tests for new algorithms
TEST_F(QuantumTests, AlgorithmErrorHandling) {
    std::string invalidKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    // Test BIKE error handling
    EXPECT_FALSE(postQuantumAlgorithms->bikeEncrypt(message, invalidKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->bikeDecrypt(ciphertext, invalidKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->bikeEncrypt("", invalidKey, ciphertext));

    // Test HQC error handling
    EXPECT_FALSE(postQuantumAlgorithms->hqcEncrypt(message, invalidKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->hqcDecrypt(ciphertext, invalidKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->hqcEncrypt("", invalidKey, ciphertext));

    // Test SIKE error handling
    EXPECT_FALSE(postQuantumAlgorithms->sikeEncrypt(message, invalidKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->sikeDecrypt(ciphertext, invalidKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->sikeEncrypt("", invalidKey, ciphertext));
}

// Edge case tests for post-quantum algorithms
TEST_F(QuantumTests, AlgorithmEdgeCases) {
    // Test with empty keys
    std::string emptyKey;
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    
    EXPECT_FALSE(postQuantumAlgorithms->bikeEncrypt(message, emptyKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->hqcEncrypt(message, emptyKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->sikeEncrypt(message, emptyKey, ciphertext));
    
    // Test with invalid key lengths
    std::string shortKey = "short";
    EXPECT_FALSE(postQuantumAlgorithms->bikeEncrypt(message, shortKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->hqcEncrypt(message, shortKey, ciphertext));
    EXPECT_FALSE(postQuantumAlgorithms->sikeEncrypt(message, shortKey, ciphertext));
    
    // Test with maximum message length
    std::string maxMessage(1000000, 'A');  // 1MB message
    std::string publicKey, privateKey;
    
    ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(maxMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(maxMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(maxMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(maxMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(maxMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(maxMessage, decryptedMessage);
    
    // Test with special characters
    std::string specialMessage = "!@#$%^&*()_+{}|:<>?~`-=[]\\;',./";
    
    ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(specialMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(specialMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(specialMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(specialMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(specialMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(specialMessage, decryptedMessage);
    
    // Test with Unicode characters
    std::string unicodeMessage = "Hello, 世界! 🌍";
    
    ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(unicodeMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(unicodeMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(unicodeMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(unicodeMessage, decryptedMessage);
    
    ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));
    EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(unicodeMessage, publicKey, ciphertext));
    EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(unicodeMessage, decryptedMessage);
    
    // Test with corrupted ciphertext
    std::string corruptedCiphertext = ciphertext;
    corruptedCiphertext[0] ^= 1;  // Flip one bit
    
    EXPECT_FALSE(postQuantumAlgorithms->bikeDecrypt(corruptedCiphertext, privateKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->hqcDecrypt(corruptedCiphertext, privateKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->sikeDecrypt(corruptedCiphertext, privateKey, decryptedMessage));
    
    // Test with wrong private key
    std::string wrongPrivateKey = privateKey;
    wrongPrivateKey[0] ^= 1;  // Flip one bit
    
    EXPECT_FALSE(postQuantumAlgorithms->bikeDecrypt(ciphertext, wrongPrivateKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->hqcDecrypt(ciphertext, wrongPrivateKey, decryptedMessage));
    EXPECT_FALSE(postQuantumAlgorithms->sikeDecrypt(ciphertext, wrongPrivateKey, decryptedMessage));
}

// Stress tests for post-quantum algorithms
TEST_F(QuantumTests, AlgorithmStressTests) {
    const int numIterations = 1000;
    std::string publicKey, privateKey;
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    
    // Stress test BIKE
    ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));
    for (int i = 0; i < numIterations; ++i) {
        EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(message, publicKey, ciphertext));
        EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
    
    // Stress test HQC
    ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));
    for (int i = 0; i < numIterations; ++i) {
        EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(message, publicKey, ciphertext));
        EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
    
    // Stress test SIKE
    ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));
    for (int i = 0; i < numIterations; ++i) {
        EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(message, publicKey, ciphertext));
        EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// Memory leak tests for post-quantum algorithms
TEST_F(QuantumTests, AlgorithmMemoryTests) {
    const int numIterations = 1000;
    std::string publicKey, privateKey;
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;
    
    // Test BIKE memory usage
    {
        ASSERT_TRUE(postQuantumAlgorithms->generateBikeKeyPair(publicKey, privateKey));
        for (int i = 0; i < numIterations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->bikeEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->bikeDecrypt(ciphertext, privateKey, decryptedMessage));
        }
    }
    
    // Test HQC memory usage
    {
        ASSERT_TRUE(postQuantumAlgorithms->generateHqcKeyPair(publicKey, privateKey));
        for (int i = 0; i < numIterations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->hqcEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->hqcDecrypt(ciphertext, privateKey, decryptedMessage));
        }
    }
    
    // Test SIKE memory usage
    {
        ASSERT_TRUE(postQuantumAlgorithms->generateSikeKeyPair(publicKey, privateKey));
        for (int i = 0; i < numIterations; ++i) {
            EXPECT_TRUE(postQuantumAlgorithms->sikeEncrypt(message, publicKey, ciphertext));
            EXPECT_TRUE(postQuantumAlgorithms->sikeDecrypt(ciphertext, privateKey, decryptedMessage));
        }
    }
}

TEST_CASE("NTRU Prime Tests", "[quantum][ntru_prime]") {
    PostQuantumAlgorithms algorithms;
    REQUIRE(algorithms.initialize());

    SECTION("Key Generation") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));
        REQUIRE(!publicKey.empty());
        REQUIRE(!privateKey.empty());
        REQUIRE(publicKey.size() == 761 * sizeof(int));  // NTRU Prime public key size
        REQUIRE(privateKey.size() == 2 * 761 * sizeof(int));  // NTRU Prime private key size
    }

    SECTION("Encryption and Decryption") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));

        std::string message = "Hello, NTRU Prime!";
        std::string ciphertext;
        REQUIRE(algorithms.ntruPrimeEncrypt(message, publicKey, ciphertext));
        REQUIRE(!ciphertext.empty());

        std::string decryptedMessage;
        REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
        REQUIRE(decryptedMessage == message);
    }

    SECTION("Edge Cases") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));

        // Empty message
        std::string emptyMessage;
        std::string ciphertext;
        REQUIRE_FALSE(algorithms.ntruPrimeEncrypt(emptyMessage, publicKey, ciphertext));

        // Maximum message length (1MB)
        std::string longMessage(1024 * 1024, 'A');
        REQUIRE(algorithms.ntruPrimeEncrypt(longMessage, publicKey, ciphertext));
        std::string decryptedMessage;
        REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
        REQUIRE(decryptedMessage == longMessage);

        // Special characters
        std::string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
        REQUIRE(algorithms.ntruPrimeEncrypt(specialChars, publicKey, ciphertext));
        REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
        REQUIRE(decryptedMessage == specialChars);

        // Unicode characters
        std::string unicodeMessage = "Hello, 世界!";
        REQUIRE(algorithms.ntruPrimeEncrypt(unicodeMessage, publicKey, ciphertext));
        REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
        REQUIRE(decryptedMessage == unicodeMessage);
    }

    SECTION("Error Cases") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));

        // Invalid public key
        std::string invalidPublicKey = "invalid";
        std::string message = "Test message";
        std::string ciphertext;
        REQUIRE_FALSE(algorithms.ntruPrimeEncrypt(message, invalidPublicKey, ciphertext));

        // Invalid private key
        std::string invalidPrivateKey = "invalid";
        std::string decryptedMessage;
        REQUIRE_FALSE(algorithms.ntruPrimeDecrypt(ciphertext, invalidPrivateKey, decryptedMessage));

        // Corrupted ciphertext
        std::string corruptedCiphertext = "corrupted";
        REQUIRE_FALSE(algorithms.ntruPrimeDecrypt(corruptedCiphertext, privateKey, decryptedMessage));
    }

    SECTION("Stress Test") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));

        const int numIterations = 1000;
        for (int i = 0; i < numIterations; ++i) {
            std::string message = "Test message " + std::to_string(i);
            std::string ciphertext;
            REQUIRE(algorithms.ntruPrimeEncrypt(message, publicKey, ciphertext));

            std::string decryptedMessage;
            REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
            REQUIRE(decryptedMessage == message);
        }
    }

    SECTION("Memory Test") {
        std::string publicKey, privateKey;
        REQUIRE(algorithms.generateNtruPrimeKeyPair(publicKey, privateKey));

        const int numIterations = 10000;
        for (int i = 0; i < numIterations; ++i) {
            std::string message = "Test message " + std::to_string(i);
            std::string ciphertext;
            REQUIRE(algorithms.ntruPrimeEncrypt(message, publicKey, ciphertext));

            std::string decryptedMessage;
            REQUIRE(algorithms.ntruPrimeDecrypt(ciphertext, privateKey, decryptedMessage));
            REQUIRE(decryptedMessage == message);
        }
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 