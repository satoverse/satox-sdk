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
#include "satox/core/transaction_manager.hpp"
#include <thread>
#include <future>
#include <random>
#include <chrono>

using namespace satox::core;

class TransactionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &TransactionManager::getInstance();
        manager->initialize(nlohmann::json::object());
    }

    void TearDown() override {
        manager->shutdown();
    }

    TransactionManager* manager;
};

// Initialization Tests
TEST_F(TransactionManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Already initialized
}

// Transaction Creation Tests
TEST_F(TransactionManagerTest, CreateTransaction) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->createTransaction(transaction));
    EXPECT_FALSE(manager->createTransaction(transaction)); // Duplicate hash
}

// Transaction Signing Tests
TEST_F(TransactionManagerTest, SignTransaction) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    EXPECT_TRUE(manager->signTransaction("tx123"));
    EXPECT_EQ(manager->getTransaction("tx123").state, TransactionState::SIGNED);
}

// Transaction Broadcasting Tests
TEST_F(TransactionManagerTest, BroadcastTransaction) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    manager->signTransaction("tx123");
    EXPECT_TRUE(manager->broadcastTransaction("tx123"));
    EXPECT_EQ(manager->getTransaction("tx123").state, TransactionState::BROADCAST);
}

// Transaction Confirmation Tests
TEST_F(TransactionManagerTest, ConfirmTransaction) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    manager->signTransaction("tx123");
    manager->broadcastTransaction("tx123");
    EXPECT_TRUE(manager->confirmTransaction("tx123", 1000));
    EXPECT_EQ(manager->getTransaction("tx123").state, TransactionState::CONFIRMED);
    EXPECT_EQ(manager->getTransaction("tx123").confirmationHeight, 1000);
}

// Transaction Failure Tests
TEST_F(TransactionManagerTest, FailTransaction) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    EXPECT_TRUE(manager->failTransaction("tx123", "Test error"));
    EXPECT_EQ(manager->getTransaction("tx123").state, TransactionState::FAILED);
    EXPECT_EQ(manager->getTransaction("tx123").errorMessage, "Test error");
}

// Transaction Query Tests
TEST_F(TransactionManagerTest, QueryTransactions) {
    Transaction transaction1{
        "tx1",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    Transaction transaction2{
        "tx2",
        TransactionType::ASSET_TRANSFER,
        1,
        0,
        {{"input2", 0, "script2", 2000, nlohmann::json::object()}},
        {{"address2", 2000, "script2", nlohmann::json::object()}},
        TransactionState::SIGNED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction1);
    manager->createTransaction(transaction2);

    auto txsByType = manager->getTransactionsByType(TransactionType::REGULAR);
    EXPECT_EQ(txsByType.size(), 1);

    auto txsByState = manager->getTransactionsByState(TransactionState::SIGNED);
    EXPECT_EQ(txsByState.size(), 1);

    auto txsByAddress = manager->getTransactionsByAddress("address1");
    EXPECT_EQ(txsByAddress.size(), 1);
}

// Transaction Validation Tests
TEST_F(TransactionManagerTest, ValidateTransaction) {
    // Valid transaction
    Transaction validTransaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->validateTransaction(validTransaction));

    // Invalid transaction (empty hash)
    Transaction invalidTransaction = validTransaction;
    invalidTransaction.hash = "";
    EXPECT_FALSE(manager->validateTransaction(invalidTransaction));
}

// Transaction Statistics Tests
TEST_F(TransactionManagerTest, Statistics) {
    manager->enableStats(true);

    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    manager->signTransaction("tx123");

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalTransactions, 1);
    EXPECT_EQ(stats.signedTransactions, 1);
}

// Callback Tests
TEST_F(TransactionManagerTest, Callbacks) {
    bool transactionCallbackCalled = false;
    bool stateCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerTransactionCallback([&](const Transaction& transaction) {
        transactionCallbackCalled = true;
    });

    manager->registerStateCallback([&](const std::string& txHash, TransactionState state) {
        stateCallbackCalled = true;
    });

    manager->registerErrorCallback([&](const std::string& txHash, const std::string& error) {
        errorCallbackCalled = true;
    });

    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    manager->signTransaction("tx123");
    manager->failTransaction("tx123", "Test error");

    EXPECT_TRUE(transactionCallbackCalled);
    EXPECT_TRUE(stateCallbackCalled);
    EXPECT_TRUE(errorCallbackCalled);
}

// Error Handling Tests
TEST_F(TransactionManagerTest, ErrorHandling) {
    EXPECT_FALSE(manager->signTransaction("nonexistent"));
    EXPECT_FALSE(manager->getLastError().empty());
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(TransactionManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numTransactionsPerThread = 100;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, numTransactionsPerThread]() {
            for (int j = 0; j < numTransactionsPerThread; ++j) {
                Transaction transaction{
                    "tx" + std::to_string(i) + "_" + std::to_string(j),
                    TransactionType::REGULAR,
                    1,
                    0,
                    {{"input" + std::to_string(i) + "_" + std::to_string(j), 0, "script1", 1000, nlohmann::json::object()}},
                    {{"address" + std::to_string(i) + "_" + std::to_string(j), 1000, "script1", nlohmann::json::object()}},
                    TransactionState::CREATED,
                    std::chrono::system_clock::now(),
                    std::chrono::system_clock::now(),
                    std::chrono::system_clock::now(),
                    0,
                    "",
                    nlohmann::json::object()
                };

                manager->createTransaction(transaction);
            }
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalTransactions, numThreads * numTransactionsPerThread);
}

// Edge Cases Tests
TEST_F(TransactionManagerTest, EdgeCases) {
    // Empty transaction
    Transaction emptyTransaction{
        "",
        TransactionType::REGULAR,
        0,
        0,
        {},
        {},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->validateTransaction(emptyTransaction));

    // Invalid input
    Transaction invalidInputTransaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"", 0, "", 0, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->validateTransaction(invalidInputTransaction));
}

// Cleanup Tests
TEST_F(TransactionManagerTest, Cleanup) {
    Transaction transaction{
        "tx123",
        TransactionType::REGULAR,
        1,
        0,
        {{"input1", 0, "script1", 1000, nlohmann::json::object()}},
        {{"address1", 1000, "script1", nlohmann::json::object()}},
        TransactionState::CREATED,
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        std::chrono::system_clock::now(),
        0,
        "",
        nlohmann::json::object()
    };

    manager->createTransaction(transaction);
    manager->shutdown();
    EXPECT_FALSE(manager->isTransactionExists("tx123"));
}

// Stress Tests
TEST_F(TransactionManagerTest, StressTest) {
    const int numTransactions = 10000;
    std::vector<std::string> transactionHashes;

    for (int i = 0; i < numTransactions; ++i) {
        Transaction transaction{
            "tx" + std::to_string(i),
            TransactionType::REGULAR,
            1,
            0,
            {{"input" + std::to_string(i), 0, "script1", 1000, nlohmann::json::object()}},
            {{"address" + std::to_string(i), 1000, "script1", nlohmann::json::object()}},
            TransactionState::CREATED,
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            0,
            "",
            nlohmann::json::object()
        };

        manager->createTransaction(transaction);
        transactionHashes.push_back(transaction.hash);
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalTransactions, numTransactions);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 