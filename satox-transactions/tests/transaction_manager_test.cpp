#include <gtest/gtest.h>
#include "satox/transactions/transaction_manager.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

using namespace satox::transactions;
using Transaction = TransactionManager::Transaction;

class TransactionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a test configuration
        config_ = nlohmann::json::object();
        config_["network"] = "testnet";
        config_["maxTransactions"] = 1000;
    }

    void TearDown() override {
        // Clean up after each test
        TransactionManager::getInstance().shutdown();
    }

    nlohmann::json config_;
};

TEST_F(TransactionManagerTest, SingletonInstance) {
    auto& instance1 = TransactionManager::getInstance();
    auto& instance2 = TransactionManager::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TransactionManagerTest, Initialization) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));
}

TEST_F(TransactionManagerTest, DoubleInitialization) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));
    EXPECT_FALSE(manager.initialize(config_));
    EXPECT_EQ(manager.getLastError(), "TransactionManager already initialized");
}

TEST_F(TransactionManagerTest, TransactionCreation) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    std::string transactionId;
    EXPECT_TRUE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));
    EXPECT_FALSE(transactionId.empty());

    Transaction transaction;
    EXPECT_TRUE(manager.getTransaction(transactionId, transaction));
    EXPECT_EQ(transaction.from, "sender123");
    EXPECT_EQ(transaction.to, "receiver456");
    EXPECT_EQ(transaction.amount, 1000);
    EXPECT_EQ(transaction.assetId, "asset789");
    EXPECT_EQ(transaction.status, TransactionManager::Status::PENDING);
}

TEST_F(TransactionManagerTest, TransactionSigning) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    std::string transactionId;
    EXPECT_TRUE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));

    // Test signing with a valid private key
    std::string privateKey = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    EXPECT_TRUE(manager.signTransaction(transactionId, privateKey));

    Transaction transaction;
    EXPECT_TRUE(manager.getTransaction(transactionId, transaction));
    EXPECT_FALSE(transaction.signature.empty());
}

TEST_F(TransactionManagerTest, TransactionBroadcasting) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    std::string transactionId;
    EXPECT_TRUE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));

    // Try to broadcast unsigned transaction
    EXPECT_FALSE(manager.broadcastTransaction(transactionId));
    EXPECT_EQ(manager.getLastError(), "Transaction is not signed");

    // Sign and broadcast
    std::string privateKey = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    EXPECT_TRUE(manager.signTransaction(transactionId, privateKey));
    EXPECT_TRUE(manager.broadcastTransaction(transactionId));

    Transaction transaction;
    EXPECT_TRUE(manager.getTransaction(transactionId, transaction));
    EXPECT_EQ(transaction.status, TransactionManager::Status::CONFIRMED);
}

TEST_F(TransactionManagerTest, TransactionCancellation) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    std::string transactionId;
    EXPECT_TRUE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));

    EXPECT_TRUE(manager.cancelTransaction(transactionId));

    Transaction transaction;
    EXPECT_TRUE(manager.getTransaction(transactionId, transaction));
    EXPECT_EQ(transaction.status, TransactionManager::Status::CANCELLED);
}

TEST_F(TransactionManagerTest, TransactionHistory) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Create multiple transactions
    std::vector<std::string> transactionIds;
    for (int i = 0; i < 5; ++i) {
        std::string transactionId;
        EXPECT_TRUE(manager.createTransaction(
            "sender123",
            "receiver456",
            1000 + i,
            "asset789",
            nlohmann::json::object(),
            transactionId
        ));
        transactionIds.push_back(transactionId);
    }

    // Get transaction history for sender
    std::vector<Transaction> transactions;
    EXPECT_TRUE(manager.getTransactionHistory("sender123", transactions));
    EXPECT_EQ(transactions.size(), 5);

    // Verify transactions are sorted by timestamp
    for (size_t i = 1; i < transactions.size(); ++i) {
        EXPECT_GE(transactions[i-1].timestamp, transactions[i].timestamp);
    }
}

TEST_F(TransactionManagerTest, TransactionValidation) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Test invalid addresses
    std::string transactionId;
    EXPECT_FALSE(manager.createTransaction(
        "",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));
    EXPECT_EQ(manager.getLastError(), "Invalid addresses");

    // Test invalid amount
    EXPECT_FALSE(manager.createTransaction(
        "sender123",
        "receiver456",
        0,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));
    EXPECT_EQ(manager.getLastError(), "Invalid amount");

    // Test invalid asset ID
    EXPECT_FALSE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "",
        nlohmann::json::object(),
        transactionId
    ));
    EXPECT_EQ(manager.getLastError(), "Invalid asset ID");
}

TEST_F(TransactionManagerTest, TransactionCallbacks) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    std::string transactionId;
    EXPECT_TRUE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));

    // Register a callback
    Transaction receivedTransaction;
    bool callbackCalled = false;

    auto callback = [&](const Transaction& transaction) {
        receivedTransaction = transaction;
        callbackCalled = true;
    };

    manager.registerTransactionCallback(callback);

    // Sign and broadcast to trigger callback
    std::string privateKey = "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef";
    EXPECT_TRUE(manager.signTransaction(transactionId, privateKey));
    EXPECT_TRUE(manager.broadcastTransaction(transactionId));

    // Wait a bit for callback to be called
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedTransaction.id, transactionId);
    EXPECT_EQ(receivedTransaction.from, "sender123");
    EXPECT_EQ(receivedTransaction.to, "receiver456");

    // Unregister callback
    manager.unregisterTransactionCallback(callback);
}

TEST_F(TransactionManagerTest, ErrorHandling) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Test getting non-existent transaction
    Transaction transaction;
    EXPECT_FALSE(manager.getTransaction("nonexistent", transaction));
    EXPECT_EQ(manager.getLastError(), "Transaction not found");

    // Test invalid transaction ID format
    EXPECT_FALSE(manager.signTransaction("invalid-id", "private-key"));
    EXPECT_EQ(manager.getLastError(), "Invalid transaction ID");

    // Test with uninitialized manager
    TransactionManager::getInstance().shutdown();
    std::string transactionId;
    EXPECT_FALSE(manager.createTransaction(
        "sender123",
        "receiver456",
        1000,
        "asset789",
        nlohmann::json::object(),
        transactionId
    ));
    EXPECT_EQ(manager.getLastError(), "TransactionManager not initialized");

    // Test error clearing
    manager.clearLastError();
    EXPECT_TRUE(manager.getLastError().empty());
}

TEST_F(TransactionManagerTest, ConcurrentAccess) {
    auto& manager = TransactionManager::getInstance();
    EXPECT_TRUE(manager.initialize(config_));

    // Create multiple threads that access the manager concurrently
    std::vector<std::thread> threads;
    std::vector<std::string> transactionIds;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&, i]() {
            std::string transactionId;
            EXPECT_TRUE(manager.createTransaction(
                "sender" + std::to_string(i),
                "receiver" + std::to_string(i),
                1000 + i,
                "asset" + std::to_string(i),
                nlohmann::json::object(),
                transactionId
            ));
            transactionIds.push_back(transactionId);
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all transactions were created
    EXPECT_EQ(transactionIds.size(), 10);
    for (const auto& id : transactionIds) {
        Transaction transaction;
        EXPECT_TRUE(manager.getTransaction(id, transaction));
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 