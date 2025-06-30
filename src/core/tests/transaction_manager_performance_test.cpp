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
#include <vector>
#include <random>
#include <memory>
#include "transaction_manager.hpp"

using namespace satox::core;
using namespace std::chrono;

class TransactionManagerPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
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

// Transaction Creation Performance Tests
TEST_F(TransactionManagerPerformanceTest, TransactionCreationPerformance) {
    const int num_transactions = 1000;
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };
    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_transactions; ++i) {
        auto tx = manager->createTransaction(inputs, outputs, 0);
        ASSERT_TRUE(tx.has_value());
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double tps = (num_transactions * 1000.0) / duration.count();
    EXPECT_GE(tps, 100.0); // Expect at least 100 transactions per second
}

// Transaction Signing Performance Tests
TEST_F(TransactionManagerPerformanceTest, TransactionSigningPerformance) {
    const int num_transactions = 1000;
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };
    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_transactions; ++i) {
        auto tx = manager->createTransaction(inputs, outputs, 0);
        ASSERT_TRUE(tx.has_value());
        EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double tps = (num_transactions * 1000.0) / duration.count();
    EXPECT_GE(tps, 50.0); // Expect at least 50 transactions per second
}

// Transaction Validation Performance Tests
TEST_F(TransactionManagerPerformanceTest, TransactionValidationPerformance) {
    const int num_transactions = 1000;
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };
    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    std::vector<TransactionManager::Transaction> transactions;
    for (int i = 0; i < num_transactions; ++i) {
        auto tx = manager->createTransaction(inputs, outputs, 0);
        ASSERT_TRUE(tx.has_value());
        EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
        transactions.push_back(*tx);
    }

    auto start = high_resolution_clock::now();
    for (const auto& tx : transactions) {
        EXPECT_TRUE(manager->validateTransaction(tx));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double tps = (num_transactions * 1000.0) / duration.count();
    EXPECT_GE(tps, 200.0); // Expect at least 200 validations per second
}

// UTXO Management Performance Tests
TEST_F(TransactionManagerPerformanceTest, UTXOManagementPerformance) {
    const int num_utxos = 10000;
    std::vector<TransactionManager::UTXO> utxos;

    // Create UTXOs
    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_utxos; ++i) {
        TransactionManager::UTXO utxo{
            "tx_hash" + std::to_string(i),
            i,
            1000000,
            {0x76, 0xa9, 0x14, 0x00},
            1000,
            false,
            ""
        };
        EXPECT_TRUE(manager->updateUTXO(utxo));
        utxos.push_back(utxo);
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double utxos_per_second = (num_utxos * 1000.0) / duration.count();
    EXPECT_GE(utxos_per_second, 1000.0); // Expect at least 1000 UTXOs per second

    // Retrieve UTXOs
    start = high_resolution_clock::now();
    for (const auto& utxo : utxos) {
        auto retrieved = manager->getUTXO(utxo.tx_hash, utxo.output_index);
        ASSERT_TRUE(retrieved.has_value());
        EXPECT_EQ(retrieved->amount, utxo.amount);
    }
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start);
    
    utxos_per_second = (num_utxos * 1000.0) / duration.count();
    EXPECT_GE(utxos_per_second, 5000.0); // Expect at least 5000 retrievals per second
}

// Mempool Management Performance Tests
TEST_F(TransactionManagerPerformanceTest, MempoolManagementPerformance) {
    const int num_transactions = 1000;
    std::vector<TransactionManager::Input> inputs = {
        {"prev_tx_hash1", 0, {}, 0xffffffff}
    };
    std::vector<TransactionManager::Output> outputs = {
        {1000000, {0x76, 0xa9, 0x14, 0x00}}
    };

    std::vector<TransactionManager::Transaction> transactions;
    for (int i = 0; i < num_transactions; ++i) {
        auto tx = manager->createTransaction(inputs, outputs, 0);
        ASSERT_TRUE(tx.has_value());
        EXPECT_TRUE(manager->signTransaction(*tx, "test_private_key"));
        transactions.push_back(*tx);
    }

    // Submit transactions to mempool
    auto start = high_resolution_clock::now();
    for (auto& tx : transactions) {
        EXPECT_TRUE(manager->submitTransaction(tx));
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double tps = (num_transactions * 1000.0) / duration.count();
    EXPECT_GE(tps, 100.0); // Expect at least 100 submissions per second

    // Get mempool
    start = high_resolution_clock::now();
    auto mempool = manager->getMempool();
    end = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(end - start);
    
    EXPECT_EQ(mempool.size(), num_transactions);
    EXPECT_LE(duration.count(), 100); // Expect mempool retrieval in less than 100ms
}

// Parallel Processing Performance Tests
TEST_F(TransactionManagerPerformanceTest, ParallelProcessingPerformance) {
    const int num_threads = 8;
    const int transactions_per_thread = 100;
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    auto worker = [&]() {
        std::vector<TransactionManager::Input> inputs = {
            {"prev_tx_hash1", 0, {}, 0xffffffff}
        };
        std::vector<TransactionManager::Output> outputs = {
            {1000000, {0x76, 0xa9, 0x14, 0x00}}
        };

        for (int i = 0; i < transactions_per_thread; ++i) {
            auto tx = manager->createTransaction(inputs, outputs, 0);
            if (tx && manager->signTransaction(*tx, "test_private_key") &&
                manager->validateTransaction(*tx) && manager->submitTransaction(*tx)) {
                success_count++;
            }
        }
    };

    auto start = high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    
    double tps = (success_count * 1000.0) / duration.count();
    EXPECT_GE(tps, 100.0); // Expect at least 100 transactions per second
    EXPECT_GE(success_count, num_threads * transactions_per_thread * 0.9); // 90% success rate
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 