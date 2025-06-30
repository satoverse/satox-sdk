#include <gtest/gtest.h>
#include "satox/transactions/transaction_signer.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace satox::transactions;
using Transaction = TransactionManager::Transaction;

class TransactionSignerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the signer
        EXPECT_TRUE(TransactionSigner::getInstance().initialize());
    }

    void TearDown() override {
        // Clean up after each test
        TransactionSigner::getInstance().shutdown();
    }
};

TEST_F(TransactionSignerTest, SingletonInstance) {
    auto& instance1 = TransactionSigner::getInstance();
    auto& instance2 = TransactionSigner::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TransactionSignerTest, Initialization) {
    auto& signer = TransactionSigner::getInstance();
    EXPECT_TRUE(signer.initialize());
}

TEST_F(TransactionSignerTest, DoubleInitialization) {
    auto& signer = TransactionSigner::getInstance();
    EXPECT_TRUE(signer.initialize());
    EXPECT_FALSE(signer.initialize());
    EXPECT_EQ(signer.getLastError(), "TransactionSigner already initialized");
}

TEST_F(TransactionSignerTest, GenerateKeyPair) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    EXPECT_FALSE(keyPair.privateKey.empty());
    EXPECT_FALSE(keyPair.publicKey.empty());
    EXPECT_NE(keyPair.privateKey, keyPair.publicKey);
}

TEST_F(TransactionSignerTest, SignAndVerifyTransaction) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.type = "default";

    auto result = signer.signTransaction(transaction, keyPair.privateKey);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.signature.empty());
    EXPECT_TRUE(result.error.empty());

    bool verified = signer.verifySignature(transaction, result.signature, keyPair.publicKey);
    EXPECT_TRUE(verified);
}

TEST_F(TransactionSignerTest, SignAndVerifyBatch) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    std::vector<Transaction> transactions;
    for (int i = 0; i < 3; ++i) {
        Transaction transaction;
        transaction.from = "0x1234567890123456789012345678901234567890";
        transaction.to = "0x0987654321098765432109876543210987654321";
        transaction.amount = 1000 + i;
        transaction.assetId = "TEST123";
        transaction.timestamp = std::chrono::system_clock::now();
        transaction.type = "default";
        transactions.push_back(transaction);
    }

    auto results = signer.signTransactions(transactions, keyPair.privateKey);
    EXPECT_EQ(results.size(), transactions.size());

    for (size_t i = 0; i < transactions.size(); ++i) {
        EXPECT_TRUE(results[i].success);
        EXPECT_FALSE(results[i].signature.empty());
        EXPECT_TRUE(results[i].error.empty());

        bool verified = signer.verifySignature(transactions[i], results[i].signature, keyPair.publicKey);
        EXPECT_TRUE(verified);
    }
}

TEST_F(TransactionSignerTest, InvalidPrivateKey) {
    auto& signer = TransactionSigner::getInstance();
    
    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.type = "default";

    auto result = signer.signTransaction(transaction, "invalid_private_key");
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.signature.empty());
    EXPECT_FALSE(result.error.empty());
}

TEST_F(TransactionSignerTest, InvalidPublicKey) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.type = "default";

    auto result = signer.signTransaction(transaction, keyPair.privateKey);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.signature.empty());

    bool verified = signer.verifySignature(transaction, result.signature, "invalid_public_key");
    EXPECT_FALSE(verified);
}

TEST_F(TransactionSignerTest, InvalidSignature) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.type = "default";

    bool verified = signer.verifySignature(transaction, "invalid_signature", keyPair.publicKey);
    EXPECT_FALSE(verified);
}

TEST_F(TransactionSignerTest, ModifiedTransaction) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();
    
    Transaction transaction;
    transaction.from = "0x1234567890123456789012345678901234567890";
    transaction.to = "0x0987654321098765432109876543210987654321";
    transaction.amount = 1000;
    transaction.assetId = "TEST123";
    transaction.timestamp = std::chrono::system_clock::now();
    transaction.type = "default";

    auto result = signer.signTransaction(transaction, keyPair.privateKey);
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.signature.empty());

    // Modify the transaction
    transaction.amount = 2000;

    bool verified = signer.verifySignature(transaction, result.signature, keyPair.publicKey);
    EXPECT_FALSE(verified);
}

TEST_F(TransactionSignerTest, ErrorHandling) {
    auto& signer = TransactionSigner::getInstance();
    
    // Try operations before initialization
    Transaction transaction;
    auto result = signer.signTransaction(transaction, "private_key");
    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.error, "Not initialized");

    signer.clearLastError();
    EXPECT_TRUE(signer.getLastError().empty());
}

TEST_F(TransactionSignerTest, ConcurrentAccess) {
    auto& signer = TransactionSigner::getInstance();
    auto keyPair = signer.generateKeyPair();

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            Transaction transaction;
            transaction.from = "0x1234567890123456789012345678901234567890";
            transaction.to = "0x0987654321098765432109876543210987654321";
            transaction.amount = 1000;
            transaction.assetId = "TEST123";
            transaction.timestamp = std::chrono::system_clock::now();
            transaction.type = "default";

            auto result = signer.signTransaction(transaction, keyPair.privateKey);
            EXPECT_TRUE(result.success);
            EXPECT_TRUE(signer.verifySignature(transaction, result.signature, keyPair.publicKey));
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 