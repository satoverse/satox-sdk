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
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include "satox/blockchain/blockchain_manager.hpp"

using namespace satox::blockchain;
using namespace std::chrono_literals;

class BlockchainManagerPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test configuration
        BlockchainConfig config;
        config.network = "testnet";
        config.rpc_url = "http://localhost:7777";
        config.chain_id = 9007;  // SATOX chain ID
        config.p2p_port = 60777; // SATOX P2P port
        config.rpc_port = 7777;  // SATOX RPC port

        // Performance configuration
        config.cache_config.max_block_cache_size = 1000;
        config.cache_config.max_tx_cache_size = 10000;
        config.cache_config.max_balance_cache_size = 100000;
        config.cache_config.block_cache_ttl = 300s;
        config.cache_config.tx_cache_ttl = 60s;
        config.cache_config.balance_cache_ttl = 30s;

        config.pool_config.max_connections = 100;
        config.pool_config.min_connections = 10;
        config.pool_config.connection_timeout = 30s;
        config.pool_config.idle_timeout = 300s;
        config.pool_config.max_retries = 3;

        config.batch_config.max_batch_size = 1000;
        config.batch_config.batch_timeout = 100ms;
        config.batch_config.max_retries = 3;

        ASSERT_TRUE(manager_.initialize(config));
        ASSERT_TRUE(manager_.start());
    }

    void TearDown() override {
        manager_.shutdown();
    }

    BlockchainManager& manager_ = BlockchainManager::getInstance();
};

// Cache Performance Tests
TEST_F(BlockchainManagerPerformanceTest, BlockCachePerformance) {
    const int num_blocks = 1000;
    std::vector<std::string> block_hashes;

    // Add blocks to cache
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_blocks; ++i) {
        BlockInfo block;
        block.hash = "block_" + std::to_string(i);
        block.height = i;
        block_hashes.push_back(block.hash);
        manager_.addBlock(block);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should add at least 100 blocks per second
    EXPECT_GE(num_blocks * 1000 / duration.count(), 100);

    // Test cache retrieval
    start = std::chrono::high_resolution_clock::now();
    for (const auto& hash : block_hashes) {
        auto block = manager_.getBlockInfo(hash);
        EXPECT_TRUE(block.has_value());
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should retrieve at least 1000 blocks per second
    EXPECT_GE(num_blocks * 1000 / duration.count(), 1000);
}

TEST_F(BlockchainManagerPerformanceTest, TransactionCachePerformance) {
    const int num_transactions = 10000;
    std::vector<std::string> tx_hashes;

    // Add transactions to cache
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_transactions; ++i) {
        TransactionInfo tx;
        tx.hash = "tx_" + std::to_string(i);
        tx_hashes.push_back(tx.hash);
        manager_.addTransaction(tx);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should add at least 1000 transactions per second
    EXPECT_GE(num_transactions * 1000 / duration.count(), 1000);

    // Test cache retrieval
    start = std::chrono::high_resolution_clock::now();
    for (const auto& hash : tx_hashes) {
        auto tx = manager_.getTransactionInfo(hash);
        EXPECT_TRUE(tx.has_value());
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should retrieve at least 5000 transactions per second
    EXPECT_GE(num_transactions * 1000 / duration.count(), 5000);
}

// Connection Pool Tests
TEST_F(BlockchainManagerPerformanceTest, ConnectionPoolPerformance) {
    const int num_connections = 100;
    std::vector<std::string> node_addresses;

    // Create test node addresses
    for (int i = 0; i < num_connections; ++i) {
        node_addresses.push_back("node_" + std::to_string(i) + ":7777");
    }

    // Test connection establishment
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& address : node_addresses) {
        manager_.addPeer(address);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should establish at least 10 connections per second
    EXPECT_GE(num_connections * 1000 / duration.count(), 10);

    // Verify connection pool size
    EXPECT_EQ(manager_.getConnectedPeers().size(), num_connections);
}

// Batch Processing Tests
TEST_F(BlockchainManagerPerformanceTest, BatchProcessingPerformance) {
    const int num_blocks = 1000;
    const int num_transactions = 10000;

    // Add blocks and transactions in batches
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_blocks; ++i) {
        BlockInfo block;
        block.hash = "block_" + std::to_string(i);
        block.height = i;
        manager_.addBlock(block);
    }

    for (int i = 0; i < num_transactions; ++i) {
        TransactionInfo tx;
        tx.hash = "tx_" + std::to_string(i);
        manager_.addTransaction(tx);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should process at least 1000 operations per second
    EXPECT_GE((num_blocks + num_transactions) * 1000 / duration.count(), 1000);
}

// Parallel Processing Tests
TEST_F(BlockchainManagerPerformanceTest, ParallelProcessingPerformance) {
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::atomic<int> success_count{0};
    std::vector<std::thread> threads;

    auto worker = [&]() {
        for (int i = 0; i < operations_per_thread; ++i) {
            BlockInfo block;
            block.hash = "block_" + std::to_string(i);
            block.height = i;
            
            TransactionInfo tx;
            tx.hash = "tx_" + std::to_string(i);

            if (manager_.addBlock(block) && manager_.addTransaction(tx)) {
                success_count++;
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double operations_per_second = (success_count * 1000.0) / duration.count();
    EXPECT_GE(operations_per_second, 1000.0); // Expect at least 1000 operations per second
    EXPECT_GE(success_count, num_threads * operations_per_thread * 0.9); // 90% success rate
}

// Memory Optimization Tests
TEST_F(BlockchainManagerPerformanceTest, MemoryOptimization) {
    const int num_blocks = 1000;
    const int num_transactions = 10000;

    // Fill up the cache
    for (int i = 0; i < num_blocks; ++i) {
        BlockInfo block;
        block.hash = "block_" + std::to_string(i);
        block.height = i;
        manager_.addBlock(block);
    }

    for (int i = 0; i < num_transactions; ++i) {
        TransactionInfo tx;
        tx.hash = "tx_" + std::to_string(i);
        manager_.addTransaction(tx);
    }

    // Get initial cache size
    size_t initial_cache_size = manager_.getCacheSize();

    // Trigger memory optimization
    manager_.optimizeMemory();

    // Get final cache size
    size_t final_cache_size = manager_.getCacheSize();

    // Cache size should be reduced
    EXPECT_LT(final_cache_size, initial_cache_size);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 