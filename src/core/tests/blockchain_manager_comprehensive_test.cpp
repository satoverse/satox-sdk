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
#include "satox/core/blockchain_manager.hpp"
#include <thread>
#include <future>
#include <random>
#include <chrono>

using namespace satox::core;

class BlockchainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &BlockchainManager::getInstance();
        manager->initialize(nlohmann::json::object());
    }

    void TearDown() override {
        manager->shutdown();
    }

    BlockchainManager* manager;
};

// Initialization Tests
TEST_F(BlockchainManagerTest, Initialization) {
    EXPECT_TRUE(manager->initialize(nlohmann::json::object()));
    EXPECT_FALSE(manager->initialize(nlohmann::json::object())); // Already initialized
}

// Connection Tests
TEST_F(BlockchainManagerTest, Connection) {
    EXPECT_TRUE(manager->connect("localhost:7777"));
    EXPECT_TRUE(manager->isConnected());
    EXPECT_TRUE(manager->disconnect());
    EXPECT_FALSE(manager->isConnected());
}

// Block Management Tests
TEST_F(BlockchainManagerTest, BlockManagement) {
    Block block{
        "hash123",
        "prevHash456",
        1,
        std::chrono::system_clock::now().time_since_epoch().count(),
        "merkleRoot789",
        1,
        0x1d00ffff,
        0,
        {"tx1", "tx2"},
        nlohmann::json::object()
    };

    manager->blocks_["hash123"] = block;

    Block retrievedBlock;
    EXPECT_TRUE(manager->getBlock("hash123", retrievedBlock));
    EXPECT_EQ(retrievedBlock.hash, block.hash);
    EXPECT_EQ(retrievedBlock.height, block.height);

    Block heightBlock;
    EXPECT_TRUE(manager->getBlockByHeight(1, heightBlock));
    EXPECT_EQ(heightBlock.hash, block.hash);

    Block latestBlock;
    EXPECT_TRUE(manager->getLatestBlock(latestBlock));
    EXPECT_EQ(latestBlock.hash, block.hash);

    auto blocks = manager->getBlocks(0, 2);
    EXPECT_EQ(blocks.size(), 1);
    EXPECT_EQ(blocks[0].hash, block.hash);
}

// Transaction Management Tests
TEST_F(BlockchainManagerTest, TransactionManagement) {
    Transaction transaction{
        "tx123",
        1,
        0,
        {"input1", "input2"},
        {"output1", "output2"},
        nlohmann::json::object()
    };

    EXPECT_TRUE(manager->broadcastTransaction(transaction));

    Transaction retrievedTx;
    EXPECT_TRUE(manager->getTransaction("tx123", retrievedTx));
    EXPECT_EQ(retrievedTx.hash, transaction.hash);
    EXPECT_EQ(retrievedTx.inputs.size(), transaction.inputs.size());
    EXPECT_EQ(retrievedTx.outputs.size(), transaction.outputs.size());
}

// Blockchain Info Tests
TEST_F(BlockchainManagerTest, BlockchainInfo) {
    auto info = manager->getInfo();
    EXPECT_EQ(info.name, "Satoxcoin");
    EXPECT_EQ(info.version, "1.0.0");
    EXPECT_EQ(info.type, BlockchainType::MAINNET);
    EXPECT_EQ(info.currentHeight, manager->getCurrentHeight());
    EXPECT_EQ(info.bestBlockHash, manager->getBestBlockHash());
}

// Blockchain Statistics Tests
TEST_F(BlockchainManagerTest, Statistics) {
    manager->enableStats(true);

    Block block{
        "hash123",
        "prevHash456",
        1,
        std::chrono::system_clock::now().time_since_epoch().count(),
        "merkleRoot789",
        1,
        0x1d00ffff,
        0,
        {"tx1", "tx2"},
        nlohmann::json::object()
    };

    manager->blocks_["hash123"] = block;

    Transaction transaction{
        "tx123",
        1,
        0,
        {"input1", "input2"},
        {"output1", "output2"},
        nlohmann::json::object()
    };

    manager->broadcastTransaction(transaction);

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalBlocks, 1);
    EXPECT_EQ(stats.totalTransactions, 3); // 1 broadcast + 2 in block
}

// Callback Tests
TEST_F(BlockchainManagerTest, Callbacks) {
    bool blockCallbackCalled = false;
    bool transactionCallbackCalled = false;
    bool stateCallbackCalled = false;
    bool errorCallbackCalled = false;

    manager->registerBlockCallback([&](const Block& block) {
        blockCallbackCalled = true;
    });

    manager->registerTransactionCallback([&](const Transaction& transaction) {
        transactionCallbackCalled = true;
    });

    manager->registerStateCallback([&](BlockchainState state) {
        stateCallbackCalled = true;
    });

    manager->registerErrorCallback([&](const std::string& error) {
        errorCallbackCalled = true;
    });

    Block block{
        "hash123",
        "prevHash456",
        1,
        std::chrono::system_clock::now().time_since_epoch().count(),
        "merkleRoot789",
        1,
        0x1d00ffff,
        0,
        {"tx1", "tx2"},
        nlohmann::json::object()
    };

    manager->blocks_["hash123"] = block;
    manager->notifyBlockChange(block);

    Transaction transaction{
        "tx123",
        1,
        0,
        {"input1", "input2"},
        {"output1", "output2"},
        nlohmann::json::object()
    };

    manager->broadcastTransaction(transaction);
    manager->updateState(BlockchainState::CONNECTED);
    manager->notifyError("Test error");

    EXPECT_TRUE(blockCallbackCalled);
    EXPECT_TRUE(transactionCallbackCalled);
    EXPECT_TRUE(stateCallbackCalled);
    EXPECT_TRUE(errorCallbackCalled);
}

// Error Handling Tests
TEST_F(BlockchainManagerTest, ErrorHandling) {
    EXPECT_FALSE(manager->getBlock("nonexistent", Block{}));
    EXPECT_FALSE(manager->getLastError().empty());
    manager->clearLastError();
    EXPECT_TRUE(manager->getLastError().empty());
}

// Concurrency Tests
TEST_F(BlockchainManagerTest, Concurrency) {
    const int numThreads = 10;
    const int numBlocksPerThread = 100;
    std::vector<std::future<void>> futures;

    for (int i = 0; i < numThreads; ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, numBlocksPerThread]() {
            for (int j = 0; j < numBlocksPerThread; ++j) {
                Block block{
                    "hash" + std::to_string(i) + "_" + std::to_string(j),
                    "prevHash" + std::to_string(i) + "_" + std::to_string(j),
                    i * numBlocksPerThread + j,
                    std::chrono::system_clock::now().time_since_epoch().count(),
                    "merkleRoot" + std::to_string(i) + "_" + std::to_string(j),
                    1,
                    0x1d00ffff,
                    0,
                    {"tx1", "tx2"},
                    nlohmann::json::object()
                };

                manager->blocks_[block.hash] = block;
            }
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalBlocks, numThreads * numBlocksPerThread);
}

// Edge Cases Tests
TEST_F(BlockchainManagerTest, EdgeCases) {
    // Empty block
    Block emptyBlock{
        "",
        "",
        0,
        0,
        "",
        0,
        0,
        0,
        {},
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->validateBlock(emptyBlock));

    // Empty transaction
    Transaction emptyTransaction{
        "",
        0,
        0,
        {},
        {},
        nlohmann::json::object()
    };

    EXPECT_FALSE(manager->validateTransaction(emptyTransaction));
}

// Cleanup Tests
TEST_F(BlockchainManagerTest, Cleanup) {
    Block block{
        "hash123",
        "prevHash456",
        1,
        std::chrono::system_clock::now().time_since_epoch().count(),
        "merkleRoot789",
        1,
        0x1d00ffff,
        0,
        {"tx1", "tx2"},
        nlohmann::json::object()
    };

    manager->blocks_["hash123"] = block;
    manager->shutdown();
    EXPECT_FALSE(manager->isConnected());
    EXPECT_TRUE(manager->blocks_.empty());
}

// Stress Tests
TEST_F(BlockchainManagerTest, StressTest) {
    const int numBlocks = 10000;
    std::vector<std::string> blockHashes;

    for (int i = 0; i < numBlocks; ++i) {
        Block block{
            "hash" + std::to_string(i),
            i > 0 ? "hash" + std::to_string(i - 1) : "",
            i,
            std::chrono::system_clock::now().time_since_epoch().count(),
            "merkleRoot" + std::to_string(i),
            1,
            0x1d00ffff,
            0,
            {"tx" + std::to_string(i)},
            nlohmann::json::object()
        };

        manager->blocks_[block.hash] = block;
        blockHashes.push_back(block.hash);
    }

    auto stats = manager->getStats();
    EXPECT_EQ(stats.totalBlocks, numBlocks);
    EXPECT_EQ(stats.totalTransactions, numBlocks);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 