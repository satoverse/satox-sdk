#include <gtest/gtest.h>
#include "../../satox-quantum/include/satox/quantum/quantum_manager.hpp"

namespace satox {
namespace quantum {
namespace tests {

class QuantumManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(QuantumManagerTest, Initialization) {
    QuantumManager manager;
    EXPECT_TRUE(manager.isInitialized());
}

TEST_F(QuantumManagerTest, KeyGeneration) {
    QuantumManager manager;
    std::string publicKey, privateKey;
    EXPECT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));
    EXPECT_FALSE(publicKey.empty());
    EXPECT_FALSE(privateKey.empty());
}

TEST_F(QuantumManagerTest, Encryption) {
    QuantumManager manager;
    std::string publicKey, privateKey;
    EXPECT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));
    std::string message = "Test message";
    std::string encrypted;
    EXPECT_TRUE(manager.encrypt(publicKey, message, encrypted));
    EXPECT_FALSE(encrypted.empty());
}

TEST_F(QuantumManagerTest, Decryption) {
    QuantumManager manager;
    std::string publicKey, privateKey;
    EXPECT_TRUE(manager.generateQuantumKeyPair(publicKey, privateKey));
    std::string message = "Test message";
    std::string encrypted;
    EXPECT_TRUE(manager.encrypt(publicKey, message, encrypted));
    std::string decrypted;
    EXPECT_TRUE(manager.decrypt(privateKey, encrypted, decrypted));
    EXPECT_EQ(message, decrypted);
}

} // namespace tests
} // namespace quantum
} // namespace satox 