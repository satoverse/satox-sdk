#include <gtest/gtest.h>
#include "satox/quantum/hybrid_encryption.hpp"

using namespace satox::quantum;

class HybridEncryptionTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(encryption.initialize());
    }

    void TearDown() override {
        encryption.shutdown();
    }

    HybridEncryption encryption;
};

TEST_F(HybridEncryptionTest, Initialization) {
    EXPECT_TRUE(encryption.isInitialized());
    EXPECT_EQ(encryption.getAlgorithm(), "CRYSTALS-Kyber + AES-256-GCM");
    EXPECT_EQ(encryption.getVersion(), "1.0.0");
}

TEST_F(HybridEncryptionTest, SessionKeyGeneration) {
    std::string sessionKey;
    EXPECT_TRUE(encryption.generateSessionKey(sessionKey));
    EXPECT_FALSE(sessionKey.empty());
}

TEST_F(HybridEncryptionTest, EncryptionDecryption) {
    std::string sessionKey;
    ASSERT_TRUE(encryption.generateSessionKey(sessionKey));

    std::string originalMessage = "Test message for hybrid encryption";
    std::string encryptedData, decryptedData;

    EXPECT_TRUE(encryption.encryptWithSessionKey(originalMessage, sessionKey, encryptedData));
    EXPECT_FALSE(encryptedData.empty());
    EXPECT_NE(encryptedData, originalMessage);

    EXPECT_TRUE(encryption.decryptWithSessionKey(encryptedData, sessionKey, decryptedData));
    EXPECT_EQ(decryptedData, originalMessage);
}

TEST_F(HybridEncryptionTest, HybridEncryptionDecryption) {
    std::string quantumPublicKey = "quantum_public_key";
    std::string quantumPrivateKey = "quantum_private_key";
    std::string classicalPublicKey = "classical_public_key";
    std::string classicalPrivateKey = "classical_private_key";

    std::string originalMessage = "Test message for hybrid encryption";
    std::string encryptedData, decryptedData;

    EXPECT_TRUE(encryption.encrypt(originalMessage, quantumPublicKey, classicalPublicKey, encryptedData));
    EXPECT_FALSE(encryptedData.empty());
    EXPECT_NE(encryptedData, originalMessage);

    EXPECT_TRUE(encryption.decrypt(encryptedData, quantumPrivateKey, classicalPrivateKey, decryptedData));
    EXPECT_EQ(decryptedData, originalMessage);
}

TEST_F(HybridEncryptionTest, KeyRotation) {
    std::string oldQuantumKey = "old_quantum_key";
    std::string oldClassicalKey = "old_classical_key";
    std::string newQuantumKey, newClassicalKey;

    EXPECT_TRUE(encryption.rotateKeys(oldQuantumKey, oldClassicalKey, newQuantumKey, newClassicalKey));
    EXPECT_FALSE(newQuantumKey.empty());
    EXPECT_FALSE(newClassicalKey.empty());
}

TEST_F(HybridEncryptionTest, Reencryption) {
    std::string oldQuantumKey = "old_quantum_key";
    std::string oldClassicalKey = "old_classical_key";
    std::string newQuantumKey = "new_quantum_key";
    std::string newClassicalKey = "new_classical_key";

    std::string originalMessage = "Test message for reencryption";
    std::string encryptedData, reencryptedData, decryptedData;

    EXPECT_TRUE(encryption.encrypt(originalMessage, oldQuantumKey, oldClassicalKey, encryptedData));
    EXPECT_TRUE(encryption.reencrypt(encryptedData, oldQuantumKey, oldClassicalKey, newQuantumKey, newClassicalKey, reencryptedData));
    EXPECT_TRUE(encryption.decrypt(reencryptedData, newQuantumKey, newClassicalKey, decryptedData));
    EXPECT_EQ(decryptedData, originalMessage);
}

TEST_F(HybridEncryptionTest, InvalidKeys) {
    std::string invalidKey = "invalid_key";
    std::string message = "Test message";
    std::string result;

    EXPECT_FALSE(encryption.encryptWithSessionKey(message, invalidKey, result));
    EXPECT_FALSE(encryption.decryptWithSessionKey(message, invalidKey, result));
}

TEST_F(HybridEncryptionTest, LargeData) {
    std::string sessionKey;
    ASSERT_TRUE(encryption.generateSessionKey(sessionKey));

    // Generate a large message (1MB)
    std::string largeMessage(1024 * 1024, 'A');
    std::string encryptedData, decryptedData;

    EXPECT_TRUE(encryption.encryptWithSessionKey(largeMessage, sessionKey, encryptedData));
    EXPECT_TRUE(encryption.decryptWithSessionKey(encryptedData, sessionKey, decryptedData));
    EXPECT_EQ(decryptedData, largeMessage);
}

TEST_F(HybridEncryptionTest, MultipleOperations) {
    std::string sessionKey;
    ASSERT_TRUE(encryption.generateSessionKey(sessionKey));

    for (int i = 0; i < 100; ++i) {
        std::string message = "Test message " + std::to_string(i);
        std::string encryptedData, decryptedData;

        EXPECT_TRUE(encryption.encryptWithSessionKey(message, sessionKey, encryptedData));
        EXPECT_TRUE(encryption.decryptWithSessionKey(encryptedData, sessionKey, decryptedData));
        EXPECT_EQ(decryptedData, message);
    }
}

TEST_F(HybridEncryptionTest, ShutdownReinitialize) {
    encryption.shutdown();
    EXPECT_FALSE(encryption.isInitialized());

    EXPECT_TRUE(encryption.initialize());
    EXPECT_TRUE(encryption.isInitialized());

    std::string sessionKey;
    EXPECT_TRUE(encryption.generateSessionKey(sessionKey));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 