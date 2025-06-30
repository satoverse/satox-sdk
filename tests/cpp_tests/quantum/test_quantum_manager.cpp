#include <gtest/gtest.h>
#include "satox/quantum/quantum_manager.hpp"

using namespace satox::quantum;

class QuantumManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(manager.initialize());
    }

    void TearDown() override {
        manager.shutdown();
    }

    QuantumManager manager;
};

TEST_F(QuantumManagerTest, Initialization) {
    EXPECT_TRUE(manager.isInitialized());
    EXPECT_EQ(manager.getAlgorithm(), "CRYSTALS-Kyber");
    EXPECT_EQ(manager.getVersion(), "1.0.0");
}

TEST_F(QuantumManagerTest, KeyPairGeneration) {
    std::string publicKey, privateKey;
    EXPECT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
}

TEST_F(QuantumManagerTest, EncryptionDecryption) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));

    std::string originalMessage = "Test message for quantum encryption";
    std::string encryptedData, decryptedData;

    EXPECT_TRUE(manager.encrypt(publicKey, originalMessage, encryptedData));
    EXPECT_FALSE(encryptedData.empty());
    EXPECT_NE(encryptedData, originalMessage);

    EXPECT_TRUE(manager.decrypt(privateKey, encryptedData, decryptedData));
    EXPECT_EQ(decryptedData, originalMessage);
}

TEST_F(QuantumManagerTest, SigningVerification) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));

    std::string message = "Test message for quantum signing";
    std::string signature;

    EXPECT_TRUE(manager.sign(privateKey, message, signature));
    EXPECT_FALSE(signature.empty());

    EXPECT_TRUE(manager.verify(publicKey, message, signature));

    // Test with modified message
    std::string modifiedMessage = message + "modified";
    EXPECT_FALSE(manager.verify(publicKey, modifiedMessage, signature));
}

TEST_F(QuantumManagerTest, InvalidKeys) {
    std::string invalidKey = "invalid_key";
    std::string message = "Test message";
    std::string result;

    EXPECT_FALSE(manager.encrypt(invalidKey, message, result));
    EXPECT_FALSE(manager.decrypt(invalidKey, message, result));
    EXPECT_FALSE(manager.sign(invalidKey, message, result));
    EXPECT_FALSE(manager.verify(invalidKey, message, result));
}

TEST_F(QuantumManagerTest, LargeData) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));

    // Generate a large message (1MB)
    std::string largeMessage(1024 * 1024, 'A');
    std::string encryptedData, decryptedData;

    EXPECT_TRUE(manager.encrypt(publicKey, largeMessage, encryptedData));
    EXPECT_TRUE(manager.decrypt(privateKey, encryptedData, decryptedData));
    EXPECT_EQ(decryptedData, largeMessage);
}

TEST_F(QuantumManagerTest, MultipleOperations) {
    std::string publicKey, privateKey;
    ASSERT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = "Test message " + std::to_string(i);
        std::string encryptedData, decryptedData, signature;

        EXPECT_TRUE(manager.encrypt(publicKey, message, encryptedData));
        EXPECT_TRUE(manager.decrypt(privateKey, encryptedData, decryptedData));
        EXPECT_EQ(decryptedData, message);

        EXPECT_TRUE(manager.sign(privateKey, message, signature));
        EXPECT_TRUE(manager.verify(publicKey, message, signature));
    }
}

TEST_F(QuantumManagerTest, ShutdownReinitialize) {
    manager.shutdown();
    EXPECT_FALSE(manager.isInitialized());

    EXPECT_TRUE(manager.initialize());
    EXPECT_TRUE(manager.isInitialized());

    std::string publicKey, privateKey;
    EXPECT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 