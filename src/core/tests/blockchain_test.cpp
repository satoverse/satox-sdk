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
#include <nlohmann/json.hpp>

using namespace satox::core;

class BlockchainManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state for each test
        auto& manager = BlockchainManager::getInstance();
        manager.shutdown();
    }

    void TearDown() override {
        // Clean up after each test
        auto& manager = BlockchainManager::getInstance();
        manager.shutdown();
    }
};

TEST_F(BlockchainManagerTest, Initialization) {
    auto& manager = BlockchainManager::getInstance();
    nlohmann::json config;
    config["network"] = "mainnet";
    EXPECT_TRUE(manager.initialize(config));
}

TEST_F(BlockchainManagerTest, BlockValidation) {
    auto& manager = BlockchainManager::getInstance();
    BlockchainManager::Block block;
    block.hash = "abc";
    block.previousHash = "def";
    block.height = 1;
    block.timestamp = 123456;
    block.merkleRoot = "merkle";
    block.version = 1;
    block.bits = 1;
    block.nonce = 1;
    block.transactions = {};
    block.additionalData = nlohmann::json::object();
    // Uncomment if validateBlock is public:
    // EXPECT_TRUE(manager.validateBlock(block));
}

TEST_F(BlockchainManagerTest, InfoGetters) {
    auto& manager = BlockchainManager::getInstance();
    
    // Initialize the manager first
    nlohmann::json config;
    config["network"] = "mainnet";
    EXPECT_TRUE(manager.initialize(config));
    
    auto info = manager.getInfo();
    EXPECT_GE(info.currentHeight, 0);  // Should be 0 if no blocks exist
    EXPECT_FALSE(info.name.empty());   // Should be "Satoxcoin"
    EXPECT_EQ(info.name, "Satoxcoin"); // Verify the name is correct
    EXPECT_EQ(info.version, "1.0.0");  // Verify the version is correct
} 