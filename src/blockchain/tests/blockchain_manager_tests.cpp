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
#include "satox/blockchain/blockchain_manager.hpp"
#include "satoxcoin/satoxcoin.h" // Official Satoxcoin library
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <memory>

using namespace satox::blockchain;
using namespace testing;

// Mock for Satoxcoin RPC client
class MockSatoxcoinRPC {
public:
    MOCK_METHOD(nlohmann::json, getBlock, (const std::string& hash), ());
    MOCK_METHOD(nlohmann::json, getBlockByHeight, (uint64_t height), ());
    MOCK_METHOD(nlohmann::json, getBestBlockHash, (), ());
    MOCK_METHOD(nlohmann::json, getBalance, (const std::string& address), ());
    MOCK_METHOD(nlohmann::json, getTransactionCount, (const std::string& address), ());
    MOCK_METHOD(nlohmann::json, sendRawTransaction, (const std::string& tx), ());
};

class BlockchainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize with Satoxcoin-specific configuration
        NetworkConfig config;
        config.network_id = "satoxcoin";
        config.chain_id = 9007; // SLIP-044 for Satoxcoin
        config.rpc_url = "http://localhost:7777"; // Satoxcoin RPC port
        config.p2p_port = 60777; // Satoxcoin P2P port
        config.timeout = 30;
        config.max_retries = 3;

        // Initialize Satoxcoin library
        satoxcoin::init();
        
        // Create mock RPC client
        mock_rpc_ = std::make_shared<MockSatoxcoinRPC>();
        
        ASSERT_TRUE(BlockchainManager::getInstance().initialize(config));
    }

    void TearDown() override {
        BlockchainManager::getInstance().shutdown();
        satoxcoin::cleanup();
    }

    std::shared_ptr<MockSatoxcoinRPC> mock_rpc_;
};

// Network Connection Tests
TEST_F(BlockchainManagerTest, ConnectToNetwork) {
    // Set up mock expectations
    EXPECT_CALL(*mock_rpc_, getBestBlockHash())
        .WillOnce(Return(nlohmann::json{{"result", "0000000000000000000000000000000000000000000000000000000000000000"}}));

    EXPECT_TRUE(BlockchainManager::getInstance().connect());
    EXPECT_TRUE(BlockchainManager::getInstance().isConnected());
}

TEST_F(BlockchainManagerTest, DisconnectFromNetwork) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    EXPECT_TRUE(BlockchainManager::getInstance().disconnect());
    EXPECT_FALSE(BlockchainManager::getInstance().isConnected());
}

// Block Operations Tests
TEST_F(BlockchainManagerTest, GetLatestBlock) {
    // Set up mock expectations
    std::string blockHash = "0000000000000000000000000000000000000000000000000000000000000000";
    nlohmann::json blockData = {
        {"hash", blockHash},
        {"previousblockhash", "0000000000000000000000000000000000000000000000000000000000000001"},
        {"merkleroot", "0000000000000000000000000000000000000000000000000000000000000002"},
        {"time", 1234567890},
        {"bits", "1d00ffff"},
        {"nonce", 12345},
        {"height", 1},
        {"tx", nlohmann::json::array()}
    };

    EXPECT_CALL(*mock_rpc_, getBestBlockHash())
        .WillOnce(Return(nlohmann::json{{"result", blockHash}}));
    EXPECT_CALL(*mock_rpc_, getBlock(blockHash))
        .WillOnce(Return(nlohmann::json{{"result", blockData}}));

    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getLatestBlock();
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->hash, blockHash);
    EXPECT_EQ(block->height, 1);
}

TEST_F(BlockchainManagerTest, GetBlockByHash) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto latestBlock = BlockchainManager::getInstance().getLatestBlock();
    ASSERT_NE(latestBlock, nullptr);
    
    auto block = BlockchainManager::getInstance().getBlockByHash(latestBlock->hash);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->hash, latestBlock->hash);
}

TEST_F(BlockchainManagerTest, GetBlockByHeight) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHeight(1);
    ASSERT_NE(block, nullptr);
    EXPECT_EQ(block->height, 1);
}

// Transaction Tests
TEST_F(BlockchainManagerTest, CreateTransaction) {
    // Set up mock expectations
    std::string address = "SXx..."; // Replace with valid test address
    EXPECT_CALL(*mock_rpc_, getTransactionCount(address))
        .WillOnce(Return(nlohmann::json{{"result", 1}}));

    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    
    auto tx = BlockchainManager::getInstance().createTransaction(
        address, // from
        address, // to
        1.0,     // value
        {}       // data
    );
    
    ASSERT_NE(tx, nullptr);
    EXPECT_EQ(tx->from, address);
    EXPECT_EQ(tx->to, address);
    EXPECT_DOUBLE_EQ(tx->value, 1.0);
}

TEST_F(BlockchainManagerTest, GetBalance) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    
    // Test address (should be a valid Satoxcoin address)
    std::string address = "SXx..."; // Replace with valid test address
    
    double balance = BlockchainManager::getInstance().getBalance(address);
    EXPECT_GE(balance, 0.0);
}

TEST_F(BlockchainManagerTest, GetNonce) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    
    // Test address (should be a valid Satoxcoin address)
    std::string address = "SXx..."; // Replace with valid test address
    
    uint64_t nonce = BlockchainManager::getInstance().getNonce(address);
    EXPECT_GE(nonce, 0);
}

// KAWPOW Tests
TEST_F(BlockchainManagerTest, KAWPOWHashComputation) {
    // Use actual Satoxcoin KAWPOW implementation
    std::vector<uint8_t> header = {0x01, 0x02, 0x03, 0x04}; // Test header
    uint64_t nonce = 12345;
    std::vector<uint8_t> hash;

    EXPECT_TRUE(BlockchainManager::getInstance().computeKAWPOWHash(header, nonce, hash));
    EXPECT_EQ(hash.size(), 32); // SHA-256 hash size

    // Verify against Satoxcoin's KAWPOW implementation
    std::vector<uint8_t> satoxcoin_hash;
    satoxcoin::compute_kawpow_hash(header.data(), header.size(), nonce, satoxcoin_hash.data());
    EXPECT_EQ(hash, satoxcoin_hash);
}

TEST_F(BlockchainManagerTest, KAWPOWHashVerification) {
    std::vector<uint8_t> header = {0x01, 0x02, 0x03, 0x04}; // Test header
    uint64_t nonce = 12345;
    std::vector<uint8_t> target(32, 0xFF); // Easy target for testing

    EXPECT_TRUE(BlockchainManager::getInstance().verifyKAWPOWHash(header, nonce, target));
}

// Block Validation Tests
TEST_F(BlockchainManagerTest, ValidateBlock) {
    // Set up mock expectations
    std::string blockHash = "0000000000000000000000000000000000000000000000000000000000000000";
    nlohmann::json blockData = {
        {"hash", blockHash},
        {"previousblockhash", "0000000000000000000000000000000000000000000000000000000000000001"},
        {"merkleroot", "0000000000000000000000000000000000000000000000000000000000000002"},
        {"time", 1234567890},
        {"bits", "1d00ffff"},
        {"nonce", 12345},
        {"height", 1},
        {"tx", nlohmann::json::array()}
    };

    EXPECT_CALL(*mock_rpc_, getBlock(blockHash))
        .WillOnce(Return(nlohmann::json{{"result", blockData}}));

    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHash(blockHash);
    ASSERT_NE(block, nullptr);
    
    // Use Satoxcoin's block validation
    EXPECT_TRUE(satoxcoin::validate_block(block->getHeader(), block->nonce));
    EXPECT_TRUE(BlockchainManager::getInstance().validateBlock(block));
}

TEST_F(BlockchainManagerTest, ValidateTransaction) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    
    // Create a test transaction
    std::string from = "SXx..."; // Replace with valid test address
    std::string to = "SXx...";   // Replace with valid test address
    double value = 1.0;
    std::vector<uint8_t> data;

    auto tx = BlockchainManager::getInstance().createTransaction(from, to, value, data);
    ASSERT_NE(tx, nullptr);
    
    EXPECT_TRUE(BlockchainManager::getInstance().validateTransaction(tx));
}

// Error Handling Tests
TEST_F(BlockchainManagerTest, InvalidNetworkConfig) {
    NetworkConfig config;
    config.network_id = "";
    config.chain_id = 0;
    config.rpc_url = "invalid_url";
    config.timeout = 0;
    config.max_retries = 0;

    EXPECT_FALSE(BlockchainManager::getInstance().initialize(config));
    EXPECT_FALSE(BlockchainManager::getInstance().getLastError().empty());
}

TEST_F(BlockchainManagerTest, InvalidBlockHash) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHash("invalid_hash");
    EXPECT_EQ(block, nullptr);
}

TEST_F(BlockchainManagerTest, InvalidBlockHeight) {
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHeight(0);
    EXPECT_EQ(block, nullptr);
}

// Callback Tests
TEST_F(BlockchainManagerTest, BlockCallback) {
    bool callback_called = false;
    BlockchainManager::getInstance().registerBlockCallback(
        [&callback_called](const std::shared_ptr<Block>& block) {
            callback_called = true;
            EXPECT_NE(block, nullptr);
        }
    );

    // Set up mock expectations
    std::string blockHash = "0000000000000000000000000000000000000000000000000000000000000000";
    nlohmann::json blockData = {
        {"hash", blockHash},
        {"previousblockhash", "0000000000000000000000000000000000000000000000000000000000000001"},
        {"merkleroot", "0000000000000000000000000000000000000000000000000000000000000002"},
        {"time", 1234567890},
        {"bits", "1d00ffff"},
        {"nonce", 12345},
        {"height", 1},
        {"tx", nlohmann::json::array()}
    };

    EXPECT_CALL(*mock_rpc_, getBlock(blockHash))
        .WillOnce(Return(nlohmann::json{{"result", blockData}}));

    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHash(blockHash);
    ASSERT_NE(block, nullptr);
    
    // Note: This test might need adjustment based on how callbacks are triggered
    EXPECT_TRUE(callback_called);
}

TEST_F(BlockchainManagerTest, TransactionCallback) {
    bool callback_called = false;
    BlockchainManager::getInstance().registerTransactionCallback(
        [&callback_called](const std::shared_ptr<Transaction>& tx) {
            callback_called = true;
            EXPECT_NE(tx, nullptr);
        }
    );

    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    
    // Create and broadcast a test transaction
    std::string from = "SXx..."; // Replace with valid test address
    std::string to = "SXx...";   // Replace with valid test address
    double value = 1.0;
    std::vector<uint8_t> data;

    auto tx = BlockchainManager::getInstance().createTransaction(from, to, value, data);
    ASSERT_NE(tx, nullptr);
    
    if (BlockchainManager::getInstance().broadcastTransaction(tx)) {
        // Note: This test might need adjustment based on how callbacks are triggered
        EXPECT_TRUE(callback_called);
    }
}

TEST_F(BlockchainManagerTest, ErrorCallback) {
    bool callback_called = false;
    BlockchainManager::getInstance().registerErrorCallback(
        [&callback_called](const std::string& error) {
            callback_called = true;
            EXPECT_FALSE(error.empty());
        }
    );

    // Trigger an error by trying to get a block with an invalid hash
    ASSERT_TRUE(BlockchainManager::getInstance().connect());
    auto block = BlockchainManager::getInstance().getBlockByHash("invalid_hash");
    EXPECT_EQ(block, nullptr);
    
    // Note: This test might need adjustment based on how callbacks are triggered
    EXPECT_TRUE(callback_called);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 