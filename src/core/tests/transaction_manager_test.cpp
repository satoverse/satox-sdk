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
#include <gmock/gmock.h>
#include <chrono>
#include <thread>
#include <random>
#include <memory>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <nlohmann/json.hpp>

#include "transaction_manager.hpp"

using namespace satox::core;
using namespace testing;

class TransactionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize TransactionManager with test configuration
        TransactionManager::Config config;
        config.data_dir = "/tmp/satox_test";
        config.max_tx_size = 1000000;
        config.max_inputs = 1000;
        config.max_outputs = 1000;
        config.min_fee = 1000;
        config.max_fee = 1000000;
        config.fee_rate = 100;
        config.mempool_size = 100000;
        config.mempool_expiry = 3600;
        config.enable_fee_estimation = true;
        config.enable_utxo_cache = true;
        config.utxo_cache_size = 1000000;

        manager = &TransactionManager::getInstance();
        ASSERT_TRUE(manager->initialize(config));
    }

    void TearDown() override {
        manager->shutdown();
    }

    TransactionManager* manager;
};

// Basic Functionality Tests
TEST_F(TransactionManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(TransactionManager::Config()));
    EXPECT_FALSE(manager->initialize(TransactionManager::Config())); // Double initialization should fail
}

TEST_F(TransactionManagerTest, CreateValidTransaction) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}} // 0.01 SATOX
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_FALSE(tx->hash.empty());
    EXPECT_EQ(tx->inputs.size(), 1);
    EXPECT_EQ(tx->outputs.size(), 1);
}

TEST_F(TransactionManagerTest, CreateInvalidTransaction) {
    // Test with empty inputs
    std::vector<TransactionManager::Input> empty_inputs;
    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(empty_inputs, outputs, 0);
    EXPECT_FALSE(tx.has_value());
    EXPECT_FALSE(manager->getLastError().empty());

    // Test with empty outputs
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };
    std::vector<TransactionManager::Output> empty_outputs;

    tx = manager->createTransaction(inputs, empty_outputs, 0);
    EXPECT_FALSE(tx.has_value());
    EXPECT_FALSE(manager->getLastError().empty());
}

// Transaction Validation Tests
TEST_F(TransactionManagerTest, ValidateValidTransaction) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->validateTransaction(*tx));
}

TEST_F(TransactionManagerTest, ValidateInvalidTransaction) {
    // Test with output exceeding input
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000000, {0x76, 0xa9, 0x14, 0x00}} // 10 SATOX
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_FALSE(manager->validateTransaction(*tx));
}

// Transaction Signing Tests
TEST_F(TransactionManagerTest, SignValidTransaction) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    EXPECT_FALSE(tx->signature.empty());
}

TEST_F(TransactionManagerTest, SignInvalidTransaction) {
    TransactionManager::Transaction tx;
    EXPECT_FALSE(manager->signTransaction(tx, "test_private_key"));
    EXPECT_EQ(manager->getLastError(), "Invalid transaction");
}

// Transaction Broadcasting Tests
TEST_F(TransactionManagerTest, BroadcastValidTransaction) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    EXPECT_TRUE(manager->broadcastTransaction(tx->hash));
}

TEST_F(TransactionManagerTest, BroadcastInvalidTransaction) {
    EXPECT_FALSE(manager->broadcastTransaction("invalid_hash"));
    EXPECT_EQ(manager->getLastError(), "Transaction not found");
}

// Fee Management Tests
TEST_F(TransactionManagerTest, FeeCalculation) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff},
        {"prev_tx_hash2", 1, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}},
        {2000000, {0x76, 0xa9, 0x14, 0x01}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());

    uint64_t fee = manager->calculateFee(*tx);
    EXPECT_GE(fee, 1000); // min_fee
    EXPECT_LE(fee, 1000000); // max_fee
}

TEST_F(TransactionManagerTest, FeeValidation) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());

    EXPECT_TRUE(manager->validateFee(*tx));
}

// UTXO Management Tests
TEST_F(TransactionManagerTest, UTXOCreation) {
    TransactionManager::UTXO utxo{
        "tx_hash1",
        0,
        1000000,
        {0x76, 0xa9, 0x14, 0x00},
        1000,
        false,
        ""
    };

    EXPECT_TRUE(manager->updateUTXO(utxo));
}

TEST_F(TransactionManagerTest, UTXORetrieval) {
    TransactionManager::UTXO utxo{
        "tx_hash1",
        0,
        1000000,
        {0x76, 0xa9, 0x14, 0x00},
        1000,
        false,
        ""
    };

    EXPECT_TRUE(manager->updateUTXO(utxo));

    auto retrieved_utxo = manager->getUTXO("tx_hash1", 0);
    ASSERT_TRUE(retrieved_utxo.has_value());
    EXPECT_EQ(retrieved_utxo->amount, 1000000);
    EXPECT_FALSE(retrieved_utxo->spent);
}

// Mempool Management Tests
TEST_F(TransactionManagerTest, MempoolOperations) {
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    EXPECT_TRUE(manager->submitTransaction(*tx));

    auto mempool = manager->getMempool();
    EXPECT_FALSE(mempool.empty());
    EXPECT_TRUE(manager->removeFromMempool(tx->hash));
}

// Transaction History Tests
TEST_F(TransactionManagerTest, TransactionHistory) {
    std::string address = "test_address";
    auto history = manager->getTransactionHistory(address);
    EXPECT_TRUE(history.empty()); // Initially empty

    // Add a transaction
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    EXPECT_TRUE(manager->submitTransaction(*tx));

    history = manager->getTransactionHistory(address);
    EXPECT_FALSE(history.empty());
}

// Concurrency Tests
TEST_F(TransactionManagerTest, ConcurrentOperations) {
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, &mutex, &cv, &ready, i]() {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [&ready] { return ready; });

            std::vector<TransactionManager::Input> inputs = {
                {"prev_tx_hash" + std::to_string(i), 0, {}, 0xffffffff}
            };

            std::vector<TransactionManager::Output> outputs = {
                {1000000, {0x76, 0xa9, 0x14, 0x00}}
            };

            auto tx = manager->createTransaction(inputs, outputs, 0);
            ASSERT_TRUE(tx.has_value());
            EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
            EXPECT_TRUE(manager->submitTransaction(*tx));
        });
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        ready = true;
    }
    cv.notify_all();

    for (auto& thread : threads) {
        thread.join();
    }
}

// Performance Tests
TEST_F(TransactionManagerTest, Performance) {
    const int num_transactions = 1000;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_transactions; ++i) {
        std::vector<TransactionManager::Input> inputs = {
            {"prev_tx_hash" + std::to_string(i), 0, {}, 0xffffffff}
        };

        std::vector<TransactionManager::Output> outputs = {
            {1000000, {0x76, 0xa9, 0x14, 0x00}}
        };

        auto tx = manager->createTransaction(inputs, outputs, 0);
        ASSERT_TRUE(tx.has_value());
        EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
        EXPECT_TRUE(manager->submitTransaction(*tx));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Expect to process at least 100 transactions per second
    EXPECT_LT(duration.count(), num_transactions * 10);
}

// Edge Case Tests
TEST_F(TransactionManagerTest, EdgeCases) {
    // Test with maximum inputs
    std::vector<TransactionManager::Input> max_inputs(1000);
    for (size_t i = 0; i < max_inputs.size(); ++i) {
        max_inputs[i] = {"prev_tx_hash" + std::to_string(i), 0, {}, 0xffffffff};
    }

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(max_inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());

    // Test with maximum outputs
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> max_outputs(1000);
    for (size_t i = 0; i < max_outputs.size(); ++i) {
        max_outputs[i] = {1000000, {0x76, 0xa9, 0x14, 0x00}};
    }

    tx = manager->createTransaction(inputs, max_outputs, 0);
    ASSERT_TRUE(tx.has_value());
}

// Recovery Tests
TEST_F(TransactionManagerTest, Recovery) {
    // Test recovery after shutdown
    manager->shutdown();
    EXPECT_TRUE(manager->initialize(TransactionManager::Config()));

    // Test recovery after error
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };

    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto tx = manager->createTransaction(inputs, outputs, 0);
    ASSERT_TRUE(tx.has_value());
    EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    EXPECT_TRUE(manager->submitTransaction(*tx));

    // Simulate error recovery
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 