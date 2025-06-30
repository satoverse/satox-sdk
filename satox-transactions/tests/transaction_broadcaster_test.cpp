#include <gtest/gtest.h>
#include "satox/transactions/transaction_broadcaster.hpp"
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>
#include <chrono>

using namespace satox::transactions;
using Transaction = TransactionManager::Transaction;

class TransactionBroadcasterTest : public ::testing::Test {
protected:
    void SetUp() override {
        nlohmann::json config;
        config["max_queue_size"] = 1000;
        config["broadcast_interval_ms"] = 100;
        broadcaster_.initialize(config);
    }

    void TearDown() override {
        broadcaster_.shutdown();
    }

    TransactionBroadcaster& broadcaster_ = TransactionBroadcaster::getInstance();
};

TEST_F(TransactionBroadcasterTest, SingletonInstance) {
    auto& instance1 = TransactionBroadcaster::getInstance();
    auto& instance2 = TransactionBroadcaster::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TransactionBroadcasterTest, Initialization) {
    nlohmann::json config;
    config["max_queue_size"] = 1000;
    config["broadcast_interval_ms"] = 100;

    EXPECT_TRUE(broadcaster_.initialize(config));
    EXPECT_FALSE(broadcaster_.initialize(config)); // Double initialization should fail
}

TEST_F(TransactionBroadcasterTest, BroadcastTransaction) {
    Transaction transaction;
    transaction.id = "test_tx_1";
    transaction.type = "transfer";
    transaction.amount = 100.0;

    auto result = broadcaster_.broadcastTransaction(transaction);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.transactionId, transaction.id);
    EXPECT_TRUE(result.error.empty());
}

TEST_F(TransactionBroadcasterTest, BroadcastMultipleTransactions) {
    std::vector<Transaction> transactions;
    for (int i = 0; i < 5; ++i) {
        Transaction tx;
        tx.id = "test_tx_" + std::to_string(i);
        tx.type = "transfer";
        tx.amount = 100.0 * (i + 1);
        transactions.push_back(tx);
    }

    auto results = broadcaster_.broadcastTransactions(transactions);
    EXPECT_EQ(results.size(), transactions.size());

    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i].success);
        EXPECT_EQ(results[i].transactionId, transactions[i].id);
        EXPECT_TRUE(results[i].error.empty());
    }
}

TEST_F(TransactionBroadcasterTest, CallbackRegistration) {
    bool callbackCalled = false;
    auto callback = [&callbackCalled](const TransactionBroadcaster::BroadcastResult& result) {
        callbackCalled = true;
        EXPECT_TRUE(result.success);
    };

    EXPECT_TRUE(broadcaster_.registerCallback(callback));
    EXPECT_TRUE(broadcaster_.unregisterCallback(callback));
}

TEST_F(TransactionBroadcasterTest, CallbackNotification) {
    bool callbackCalled = false;
    auto callback = [&callbackCalled](const TransactionBroadcaster::BroadcastResult& result) {
        callbackCalled = true;
        EXPECT_TRUE(result.success);
    };

    broadcaster_.registerCallback(callback);

    Transaction transaction;
    transaction.id = "test_tx_1";
    broadcaster_.broadcastTransaction(transaction);

    // Wait for callback
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(callbackCalled);
}

TEST_F(TransactionBroadcasterTest, QueueSize) {
    EXPECT_EQ(broadcaster_.getQueueSize(), 0);

    Transaction transaction;
    transaction.id = "test_tx_1";
    broadcaster_.broadcastTransaction(transaction);

    EXPECT_EQ(broadcaster_.getQueueSize(), 1);
}

TEST_F(TransactionBroadcasterTest, ErrorHandling) {
    broadcaster_.shutdown();

    Transaction transaction;
    transaction.id = "test_tx_1";
    auto result = broadcaster_.broadcastTransaction(transaction);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

TEST_F(TransactionBroadcasterTest, ConcurrentAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, i, &successCount]() {
            Transaction transaction;
            transaction.id = "test_tx_" + std::to_string(i);
            auto result = broadcaster_.broadcastTransaction(transaction);
            if (result.success) {
                ++successCount;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount, 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 