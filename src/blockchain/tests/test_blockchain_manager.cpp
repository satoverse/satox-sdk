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
#include "satox/blockchain/blockchain_manager.hpp"
#include <thread>
#include <chrono>

using namespace satox::blockchain;

class BlockchainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        NetworkConfig config{
            "testnet",
            "http://localhost:8545",
            1337,
            21000,
            20000000000,
            30,
            3
        };
        manager = &BlockchainManager::getInstance();
        manager->initialize(config);
    }

    void TearDown() override {
        manager->shutdown();
    }

    BlockchainManager* manager;
};

TEST_F(BlockchainManagerTest, Initialization) {
    EXPECT_TRUE(manager->isConnected());
    EXPECT_FALSE(manager->getLastError().empty());
}

TEST_F(BlockchainManagerTest, NetworkConnection) {
    EXPECT_TRUE(manager->connect());
    EXPECT_TRUE(manager->isConnected());
    
    EXPECT_TRUE(manager->disconnect());
    EXPECT_FALSE(manager->isConnected());
}

TEST_F(BlockchainManagerTest, BlockOperations) {
    // Get latest block
    auto latest_block = manager->getLatestBlock();
    EXPECT_NE(latest_block, nullptr);
    EXPECT_FALSE(latest_block->getHash().empty());
    
    // Get block by hash
    auto block_by_hash = manager->getBlockByHash(latest_block->getHash());
    EXPECT_NE(block_by_hash, nullptr);
    EXPECT_EQ(block_by_hash->getHash(), latest_block->getHash());
    
    // Get block by height
    auto block_by_height = manager->getBlockByHeight(0);
    EXPECT_NE(block_by_height, nullptr);
}

TEST_F(BlockchainManagerTest, TransactionOperations) {
    // Create transaction
    auto tx = manager->createTransaction(
        "0x742d35Cc6634C0532925a3b844Bc454e4438f44e",
        "0x742d35Cc6634C0532925a3b844Bc454e4438f44f",
        1.0,
        std::vector<uint8_t>{0x01, 0x02, 0x03}
    );
    EXPECT_NE(tx, nullptr);
    EXPECT_FALSE(tx->getHash().empty());
    
    // Broadcast transaction
    EXPECT_TRUE(manager->broadcastTransaction(tx));
    
    // Get transaction status
    std::string status = manager->getTransactionStatus(tx->getHash());
    EXPECT_FALSE(status.empty());
    
    // Get transaction
    auto retrieved_tx = manager->getTransaction(tx->getHash());
    EXPECT_NE(retrieved_tx, nullptr);
    EXPECT_EQ(retrieved_tx->getHash(), tx->getHash());
}

TEST_F(BlockchainManagerTest, AccountOperations) {
    std::string address = "0x742d35Cc6634C0532925a3b844Bc454e4438f44e";
    
    // Get balance
    double balance = manager->getBalance(address);
    EXPECT_GE(balance, 0.0);
    
    // Get nonce
    uint64_t nonce = manager->getNonce(address);
    EXPECT_GE(nonce, 0);
}

TEST_F(BlockchainManagerTest, ErrorHandling) {
    // Test invalid address
    EXPECT_EQ(manager->getBalance(""), 0.0);
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test invalid block hash
    EXPECT_EQ(manager->getBlockByHash(""), nullptr);
    EXPECT_FALSE(manager->getLastError().empty());
    
    // Test invalid transaction hash
    EXPECT_EQ(manager->getTransaction(""), nullptr);
    EXPECT_FALSE(manager->getLastError().empty());
}

TEST_F(BlockchainManagerTest, EventCallbacks) {
    bool block_callback_called = false;
    bool tx_callback_called = false;
    bool error_callback_called = false;
    
    manager->registerBlockCallback([&](const std::shared_ptr<Block>& block) {
        block_callback_called = true;
    });
    
    manager->registerTransactionCallback([&](const std::shared_ptr<Transaction>& tx) {
        tx_callback_called = true;
    });
    
    manager->registerErrorCallback([&](const std::string& error) {
        error_callback_called = true;
    });
    
    // Trigger callbacks
    manager->getLatestBlock();
    manager->createTransaction(
        "0x742d35Cc6634C0532925a3b844Bc454e4438f44e",
        "0x742d35Cc6634C0532925a3b844Bc454e4438f44f",
        1.0,
        std::vector<uint8_t>{}
    );
    manager->getBalance("");
    
    // Wait for callbacks
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(block_callback_called);
    EXPECT_TRUE(tx_callback_called);
    EXPECT_TRUE(error_callback_called);
}

TEST_F(BlockchainManagerTest, NetworkConfig) {
    NetworkConfig new_config{
        "mainnet",
        "http://localhost:8546",
        1,
        21000,
        20000000000,
        30,
        3
    };
    
    EXPECT_TRUE(manager->updateNetworkConfig(new_config));
    auto current_config = manager->getNetworkConfig();
    EXPECT_EQ(current_config.network_id, new_config.network_id);
    EXPECT_EQ(current_config.rpc_url, new_config.rpc_url);
    EXPECT_EQ(current_config.chain_id, new_config.chain_id);
}

TEST_F(BlockchainManagerTest, ThreadSafety) {
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this]() {
            manager->getLatestBlock();
            manager->getBalance("0x742d35Cc6634C0532925a3b844Bc454e4438f44e");
            manager->getNonce("0x742d35Cc6634C0532925a3b844Bc454e4438f44e");
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
} 