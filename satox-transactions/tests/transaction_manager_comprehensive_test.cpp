#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "satox/transactions/transaction_manager.hpp"
#include <thread>
#include <future>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>

using namespace satox::transactions;
using namespace std::chrono_literals;

class TransactionManagerComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Transaction Manager with test configuration
        nlohmann::json config = {
            {"fee_rate", 1},
            {"max_mempool_size", 1000},
            {"confirmation_timeout", 300},
            {"min_fee", 1000}
        };
        ASSERT_TRUE(TransactionManager::getInstance().initialize(config));
    }

    void TearDown() override {
        TransactionManager::getInstance().shutdown();
    }

    // Helper function to create a test transaction
    Transaction createTestTransaction(const std::string& from, const std::string& to,
                                    uint64_t amount, const std::string& assetId = "SAT") {
        Transaction tx;
        tx.id = "test_tx_" + std::to_string(std::rand());
        tx.from = from;
        tx.to = to;
        tx.amount = amount;
        tx.assetId = assetId;
        tx.status = Status::PENDING;
        tx.timestamp = std::chrono::system_clock::now();
        tx.fee = 1000;
        return tx;
    }

    // Helper function to create a test UTXO
    UTXO createTestUTXO(const std::string& address, uint64_t amount,
                       const std::string& assetId = "SAT") {
        UTXO utxo;
        utxo.txId = "test_utxo_" + std::to_string(std::rand());
        utxo.outputIndex = 0;
        utxo.amount = amount;
        utxo.assetId = assetId;
        utxo.address = address;
        utxo.timestamp = std::chrono::system_clock::now();
        utxo.spent = false;
        return utxo;
    }
};

// Basic Functionality Tests
TEST_F(TransactionManagerComprehensiveTest, Initialization) {
    // Test initialization with valid config
    nlohmann::json validConfig = {
        {"fee_rate", 1},
        {"max_mempool_size", 1000}
    };
    EXPECT_TRUE(TransactionManager::getInstance().initialize(validConfig));

    // Test initialization with invalid config
    nlohmann::json invalidConfig = {
        {"invalid_key", "invalid_value"}
    };
    EXPECT_FALSE(TransactionManager::getInstance().initialize(invalidConfig));
}

// Transaction Operations Tests
TEST_F(TransactionManagerComprehensiveTest, TransactionCreation) {
    std::string txId;
    nlohmann::json metadata = {{"description", "Test transaction"}};
    
    // Test valid transaction creation
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", metadata, txId));
    EXPECT_FALSE(txId.empty());

    // Test transaction retrieval
    Transaction tx;
    EXPECT_TRUE(TransactionManager::getInstance().getTransaction(txId, tx));
    EXPECT_EQ(tx.from, "address1");
    EXPECT_EQ(tx.to, "address2");
    EXPECT_EQ(tx.amount, 1000);
    EXPECT_EQ(tx.assetId, "SAT");
    EXPECT_EQ(tx.status, Status::PENDING);
}

TEST_F(TransactionManagerComprehensiveTest, TransactionSigning) {
    std::string txId;
    nlohmann::json metadata = {{"description", "Test transaction"}};
    
    // Create and sign transaction
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", metadata, txId));
    EXPECT_TRUE(TransactionManager::getInstance().signTransaction(txId, "test_private_key"));

    // Verify signature
    Transaction tx;
    EXPECT_TRUE(TransactionManager::getInstance().getTransaction(txId, tx));
    EXPECT_FALSE(tx.signature.empty());
}

TEST_F(TransactionManagerComprehensiveTest, TransactionBroadcasting) {
    std::string txId;
    nlohmann::json metadata = {{"description", "Test transaction"}};
    
    // Create and broadcast transaction
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", metadata, txId));
    EXPECT_TRUE(TransactionManager::getInstance().broadcastTransaction(txId));

    // Verify transaction status
    Status status;
    EXPECT_TRUE(TransactionManager::getInstance().getTransactionStatus(txId, status));
    EXPECT_EQ(status, Status::CONFIRMED);
}

// UTXO Operations Tests
TEST_F(TransactionManagerComprehensiveTest, UTXOManagement) {
    // Add UTXO
    UTXO utxo = createTestUTXO("address1", 1000);
    EXPECT_TRUE(TransactionManager::getInstance().addUTXO(utxo));

    // Get UTXOs for address
    std::vector<UTXO> utxos;
    EXPECT_TRUE(TransactionManager::getInstance().getUTXOs("address1", utxos));
    EXPECT_FALSE(utxos.empty());
    EXPECT_EQ(utxos[0].amount, 1000);

    // Spend UTXO
    EXPECT_TRUE(TransactionManager::getInstance().spendUTXO(utxo.txId, utxo.outputIndex));
    
    // Verify UTXO is spent
    utxos.clear();
    EXPECT_TRUE(TransactionManager::getInstance().getUTXOs("address1", utxos));
    EXPECT_TRUE(utxos.empty());
}

// Fee Operations Tests
TEST_F(TransactionManagerComprehensiveTest, FeeCalculation) {
    Transaction tx = createTestTransaction("address1", "address2", 1000);
    
    // Test fee calculation
    uint64_t fee = TransactionManager::getInstance().calculateFee(tx);
    EXPECT_GT(fee, 0);

    // Test fee estimation
    uint64_t estimatedFee = TransactionManager::getInstance().estimateFee(1, 1);
    EXPECT_GT(estimatedFee, 0);

    // Test fee rate setting
    EXPECT_TRUE(TransactionManager::getInstance().setFeeRate(2));
}

// Mempool Operations Tests
TEST_F(TransactionManagerComprehensiveTest, MempoolOperations) {
    Transaction tx = createTestTransaction("address1", "address2", 1000);
    
    // Add to mempool
    EXPECT_TRUE(TransactionManager::getInstance().addToMempool(tx));

    // Get mempool transactions
    std::vector<Transaction> mempoolTxs;
    EXPECT_TRUE(TransactionManager::getInstance().getMempoolTransactions(mempoolTxs));
    EXPECT_FALSE(mempoolTxs.empty());

    // Get mempool size
    size_t mempoolSize;
    EXPECT_TRUE(TransactionManager::getInstance().getMempoolSize(mempoolSize));
    EXPECT_GT(mempoolSize, 0);

    // Remove from mempool
    EXPECT_TRUE(TransactionManager::getInstance().removeFromMempool(tx.id));
}

// Transaction Monitoring Tests
TEST_F(TransactionManagerComprehensiveTest, TransactionMonitoring) {
    bool callbackCalled = false;
    TransactionCallback callback = [&callbackCalled](const Transaction& tx) {
        callbackCalled = true;
    };

    // Register callback
    TransactionManager::getInstance().registerTransactionCallback(callback);

    // Create and broadcast transaction
    std::string txId;
    nlohmann::json metadata = {{"description", "Test transaction"}};
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", metadata, txId));
    EXPECT_TRUE(TransactionManager::getInstance().broadcastTransaction(txId));

    // Wait for callback
    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(callbackCalled);

    // Unregister callback
    TransactionManager::getInstance().unregisterTransactionCallback(callback);
}

// Error Handling Tests
TEST_F(TransactionManagerComprehensiveTest, ErrorHandling) {
    // Test invalid transaction creation
    std::string txId;
    EXPECT_FALSE(TransactionManager::getInstance().createTransaction(
        "", "", 0, "", nlohmann::json(), txId));
    EXPECT_FALSE(TransactionManager::getInstance().getLastError().empty());

    // Test invalid transaction signing
    EXPECT_FALSE(TransactionManager::getInstance().signTransaction("invalid_tx_id", ""));
    EXPECT_FALSE(TransactionManager::getInstance().getLastError().empty());

    // Test invalid transaction broadcasting
    EXPECT_FALSE(TransactionManager::getInstance().broadcastTransaction("invalid_tx_id"));
    EXPECT_FALSE(TransactionManager::getInstance().getLastError().empty());

    // Clear error
    TransactionManager::getInstance().clearLastError();
    EXPECT_TRUE(TransactionManager::getInstance().getLastError().empty());
}

// Concurrency Tests
TEST_F(TransactionManagerComprehensiveTest, Concurrency) {
    const int numThreads = 10;
    std::vector<std::future<void>> futures;

    // Test concurrent transaction creation
    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            std::string txId;
            nlohmann::json metadata = {{"description", "Concurrent test " + std::to_string(i)}};
            EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
                "address1", "address2", 1000, "SAT", metadata, txId));
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }
}

// Edge Cases Tests
TEST_F(TransactionManagerComprehensiveTest, EdgeCases) {
    // Test maximum amount
    std::string txId;
    nlohmann::json metadata = {{"description", "Max amount test"}};
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", UINT64_MAX, "SAT", metadata, txId));

    // Test minimum amount
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1, "SAT", metadata, txId));

    // Test empty metadata
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", nlohmann::json(), txId));
}

// Cleanup Tests
TEST_F(TransactionManagerComprehensiveTest, Cleanup) {
    // Create and broadcast transaction
    std::string txId;
    nlohmann::json metadata = {{"description", "Cleanup test"}};
    EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
        "address1", "address2", 1000, "SAT", metadata, txId));
    EXPECT_TRUE(TransactionManager::getInstance().broadcastTransaction(txId));

    // Cancel transaction
    EXPECT_TRUE(TransactionManager::getInstance().cancelTransaction(txId));

    // Verify transaction status
    Status status;
    EXPECT_TRUE(TransactionManager::getInstance().getTransactionStatus(txId, status));
    EXPECT_EQ(status, Status::CANCELLED);
}

// Stress Tests
TEST_F(TransactionManagerComprehensiveTest, StressTest) {
    const int numTransactions = 1000;
    std::vector<std::string> txIds;

    // Create multiple transactions
    for (int i = 0; i < numTransactions; ++i) {
        std::string txId;
        nlohmann::json metadata = {{"description", "Stress test " + std::to_string(i)}};
        EXPECT_TRUE(TransactionManager::getInstance().createTransaction(
            "address1", "address2", 1000, "SAT", metadata, txId));
        txIds.push_back(txId);
    }

    // Verify all transactions
    for (const auto& txId : txIds) {
        Transaction tx;
        EXPECT_TRUE(TransactionManager::getInstance().getTransaction(txId, tx));
        EXPECT_EQ(tx.amount, 1000);
    }
} 