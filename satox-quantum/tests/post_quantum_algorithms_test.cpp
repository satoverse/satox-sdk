#include <gtest/gtest.h>
#include "satox/quantum/post_quantum_algorithms.hpp"
#include <string>
#include <vector>
#include <random>
#include <chrono>

class PostQuantumAlgorithmsTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(algorithms.initialize());
    }

    void TearDown() override {
        algorithms.shutdown();
    }

    satox::quantum::PostQuantumAlgorithms& algorithms = satox::quantum::PostQuantumAlgorithms::getInstance();

    // Helper function to generate random message
    std::string generateRandomMessage(size_t length) {
        std::string message;
        message.reserve(length);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < length; ++i) {
            message.push_back(static_cast<char>(dis(gen)));
        }
        return message;
    }

    // Helper function to measure encryption/decryption time
    template<typename Func>
    double measureTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(end - start).count();
    }
};

// CRYSTALS-Kyber Tests
TEST_F(PostQuantumAlgorithmsTest, KyberKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, KyberEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    std::string message = "Test message for Kyber encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.kyberEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, KyberLargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.kyberEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, KyberPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.kyberEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

// NTRU Tests
TEST_F(PostQuantumAlgorithmsTest, NTRUKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, NTRUEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    std::string message = "Test message for NTRU encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.ntruEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, NTRULargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.ntruEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, NTRUPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.ntruEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

// Security Tests
TEST_F(PostQuantumAlgorithmsTest, KyberKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.kyberEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.kyberEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, NTRUKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.ntruEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.ntruEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, KyberInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.kyberEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage));
}

TEST_F(PostQuantumAlgorithmsTest, NTRUInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.ntruEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage));
}

// Stress Tests
TEST_F(PostQuantumAlgorithmsTest, KyberStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.kyberEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, NTRUStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.ntruEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// Memory Tests
TEST_F(PostQuantumAlgorithmsTest, KyberMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateKyberKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.kyberEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.kyberDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, NTRUMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateNTRUKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.ntruEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.ntruDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// BIKE Tests
TEST_F(PostQuantumAlgorithmsTest, BIKEKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, BIKEEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    std::string message = "Test message for BIKE encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.bikeEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, BIKELargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.bikeEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, BIKEPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.bikeEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

// HQC Tests
TEST_F(PostQuantumAlgorithmsTest, HQCKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, HQCEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    std::string message = "Test message for HQC encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.hqcEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, HQCLargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.hqcEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, HQCPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.hqcEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

// Additional Security Tests
TEST_F(PostQuantumAlgorithmsTest, BIKEKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.bikeEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.bikeEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, HQCKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.hqcEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.hqcEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, BIKEInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.bikeEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage));
}

TEST_F(PostQuantumAlgorithmsTest, HQCInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.hqcEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage));
}

// Additional Stress Tests
TEST_F(PostQuantumAlgorithmsTest, BIKEStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.bikeEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, HQCStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.hqcEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// Additional Memory Tests
TEST_F(PostQuantumAlgorithmsTest, BIKEMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateBIKEKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.bikeEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.bikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, HQCMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateHQCKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.hqcEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.hqcDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// SIKE Tests
TEST_F(PostQuantumAlgorithmsTest, SIKEKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, SIKEEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    std::string message = "Test message for SIKE encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.sikeEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, SIKELargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.sikeEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, SIKEPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.sikeEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

TEST_F(PostQuantumAlgorithmsTest, SIKEKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.sikeEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.sikeEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, SIKEInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.sikeEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage));
}

TEST_F(PostQuantumAlgorithmsTest, SIKEStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.sikeEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, SIKEMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateSIKEKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.sikeEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.sikeDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// FrodoKEM Tests
TEST_F(PostQuantumAlgorithmsTest, FrodoKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, FrodoEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    std::string message = "Test message for FrodoKEM encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.frodoEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, FrodoLargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.frodoEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, FrodoPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.frodoEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

TEST_F(PostQuantumAlgorithmsTest, FrodoKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.frodoEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.frodoEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, FrodoInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.frodoEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage));
}

TEST_F(PostQuantumAlgorithmsTest, FrodoStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.frodoEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, FrodoMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateFrodoKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.frodoEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.frodoDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

// ThreeBears Tests
TEST_F(PostQuantumAlgorithmsTest, ThreeBearsKeyGeneration) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
    EXPECT_NE(publicKey, privateKey);
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsEncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    std::string message = "Test message for ThreeBears encryption";
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.threeBearsEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsLargeMessage) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    ASSERT_TRUE(algorithms.threeBearsEncrypt(message, publicKey, ciphertext));
    ASSERT_TRUE(algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage));
    EXPECT_EQ(message, decryptedMessage);
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsPerformance) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    std::string message = generateRandomMessage(1024);
    std::string ciphertext, decryptedMessage;

    double encryptTime = measureTime([&]() {
        algorithms.threeBearsEncrypt(message, publicKey, ciphertext);
    });

    double decryptTime = measureTime([&]() {
        algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage);
    });

    EXPECT_LT(encryptTime, 1.0);  // Encryption should take less than 1 second
    EXPECT_LT(decryptTime, 1.0);  // Decryption should take less than 1 second
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsKeyReuse) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    std::string message1 = "First message";
    std::string message2 = "Second message";
    std::string ciphertext1, ciphertext2, decryptedMessage;

    ASSERT_TRUE(algorithms.threeBearsEncrypt(message1, publicKey, ciphertext1));
    ASSERT_TRUE(algorithms.threeBearsEncrypt(message2, publicKey, ciphertext2));
    EXPECT_NE(ciphertext1, ciphertext2);  // Same key should produce different ciphertexts
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsInvalidKey) {
    std::string publicKey = "invalid_key";
    std::string privateKey = "invalid_key";
    std::string message = "Test message";
    std::string ciphertext, decryptedMessage;

    EXPECT_FALSE(algorithms.threeBearsEncrypt(message, publicKey, ciphertext));
    EXPECT_FALSE(algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage));
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsStressTest) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.threeBearsEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

TEST_F(PostQuantumAlgorithmsTest, ThreeBearsMemoryLeak) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(algorithms.generateThreeBearsKeyPair(publicKey, privateKey));

    for (int i = 0; i < 1000; ++i) {
        std::string message = generateRandomMessage(1024);
        std::string ciphertext, decryptedMessage;

        ASSERT_TRUE(algorithms.threeBearsEncrypt(message, publicKey, ciphertext));
        ASSERT_TRUE(algorithms.threeBearsDecrypt(ciphertext, privateKey, decryptedMessage));
        EXPECT_EQ(message, decryptedMessage);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 